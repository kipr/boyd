#include "boyd_runner.hpp"
#include "object.hpp"
#include "camera_config.hpp"
#include <battlecreek/channel_data.hpp>
#include <battlecreek/blob.hpp>
#include <battlecreek/settings.hpp>
#include <battlecreek/camera_config.hpp>

#include <daylite/node.hpp>
#include <daylite/spinner.hpp>
#include <daylite/util.hpp>

#include <iostream>
#include <thread>
#include <dirent.h>

using namespace daylite;
using namespace std::chrono;

void BoydRunner::run()
{ 
  // Load default config file
  const CameraConfig *const defaultConfig = new CameraConfig(ConfigPath::defaultConfigPath());
  if(!defaultConfig->isValid()) {
    std::cerr << "Failed to load the default config file" << std::endl;
    return;
  }
  camera.setConfig(defaultConfig);
    
  // Setup daylite node, pubs, and subs
  std::shared_ptr<node> node = node::create_node("boyd");
  auto res = node->start("127.0.0.1", 8374);
  if(!res) {
    std::cerr << "Failed to join daylite!" << std::endl;
    return;
  }
  
  auto settingsSub = node->subscribe(BoydRunner::setSettingsTopic, BoydRunner::receivedSettings);
  auto framePub = node->advertise(BoydRunner::frameTopic);
  auto settingsPub = node->advertise(BoydRunner::settingsTopic);
  framePub->set_firehose(true);
  settingsPub->set_firehose(true);
  
  // Try to open camera at launch
  if(camera.open(0)) {
    camera.setWidth(640);
    camera.setHeight(480);
    std::cout << "Camera connected at launch" << std::endl;
  }
  else
    std::cout << "Failed to open camera at launch" << std::endl;
  
  int frameNum = 0;
  auto startTime = std::chrono::steady_clock::now();
  
  boyd::frame_data fd;
  while(!daylite::should_exit()) {
    daylite::spinner::spin_once();
    
    // Only process/send frames if someone cares about them
    if(framePub->subscriber_count() > 0) {
      //std::cout << "Num subscribers: " << framePub->subscriber_count() << std::endl;
      
      // Try to open the camera, if not already open
      if(!camera.isOpen()) {
        if(camera.open(0)) {
          camera.setWidth(320);
          camera.setHeight(240);
          std::cout << "Camera connected" << std::endl;
        }
        else
          std::cout << "Failed to open camera" << std::endl;
      }
      
      // Try to grab a new frame
      // Only send frames if update succeeded
      if(camera.update()) {
        // Publish frame and blobs over daylite
        BoydRunner::createFrameData(fd);
        fd.frameNum = frameNum;
        framePub->publish(bson(fd.bind()));
        //std::cout << "Sent frame " << frameNum << std::endl;
      
        // Frame timing
        ++frameNum;
        auto duration = steady_clock::now() - startTime;
        if(duration >= seconds(1)) {
          //std::cout << "Published " << frameNum << " frames in " << duration_cast<milliseconds>(duration).count() << std::endl;
          //frameNum = 0;
          startTime = steady_clock::now();
        }
      }
    }
    
    // Only send settings if someone cares about them
    // TODO: Limit how often this is published?
    if(settingsPub->subscriber_count() > 0) {
      boyd::settings s;
      s.width = camera.imageWidth();
      s.height = camera.imageHeight();
      s.maxNumBlobs = BoydRunner::maxNumBlobs;
      s.config_base_path = ConfigPath::baseDir();
      // TODO: Need config name
      //s.config_name = ??
      
      // TODO: Make this cross-platform
      std::vector<std::string> confNames;
      DIR *const d = opendir(ConfigPath::sysBaseDir().c_str());
      struct dirent *dir;
      if(d) {
        while((dir = readdir(d)) != NULL) {
          std::string str(dir->d_name);
          if(str.size() > 5 && str.compare(str.size() - 5, 5, ".conf") == 0)
            confNames.push_back(str.substr(0, str.size() - 5));
        }
        closedir(d);
      }
      std::vector<boyd::camera_config> camConfigs;
      for(const std::string &confName : confNames) {
        CameraConfig cameraConfig(ConfigPath::pathToSysConfig(confName));
        if(!cameraConfig.isValid()) {
          std::cout << confName << " is not a valid config" << std::endl;
          continue;
        }
        boyd::camera_config camConfig;
        camConfig.config_name = confName;
        const int numChannels = cameraConfig.numChannels();
        for(int i = 0; i < numChannels; ++i) {
          const CameraConfig::HsvBounds hsvs = cameraConfig.channel(i);
          boyd::channel_config chConfig;
          chConfig.channel_name = cameraConfig.channelName(i);
          chConfig.bh = hsvs.bh;
          chConfig.bs = hsvs.bs;
          chConfig.bv = hsvs.bv;
          chConfig.th = hsvs.th;
          chConfig.ts = hsvs.ts;
          chConfig.tv = hsvs.tv;
          camConfig.channels.push_back(chConfig);
        }
        camConfigs.push_back(camConfig);
      }
      s.camera_configs = camConfigs;
      
      settingsPub->publish(bson(s.bind()));
    }
    
    // TODO: This is an arbitrary sleep interval for testing purposes
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
}

// Called when we receive setting changes
void BoydRunner::receivedSettings(const bson &msg, void *)
{
  using namespace boyd;
  
  std::cout << "Received settings" << std::endl;
  settings s = settings::unbind(msg);
  if(s.width.some())
    camera.setWidth(s.width.unwrap());
  if(s.height.some())
    camera.setHeight(s.height.unwrap());
  if(s.maxNumBlobs.some())
    BoydRunner::maxNumBlobs = s.maxNumBlobs.unwrap();
  if(s.config_base_path.some()) {
    const std::string &basePath = s.config_base_path.unwrap();
    if(basePath.empty())
      ConfigPath::setBaseDir(ConfigPath::sysBaseDir());
    else
      ConfigPath::setBaseDir(basePath);
  }
  if(s.config_name.some()) {
    const std::string &name = s.config_name.unwrap();
    const std::string &path = name.empty() ? ConfigPath::defaultConfigPath() : ConfigPath::pathToConfig(name);
    
    // Try to load the new config
    std::cout << "Loading new config file: " << path << std::endl;
    const CameraConfig *const newConfig = new CameraConfig(path);
    if(!newConfig->isValid()) {
      std::cerr << "New config file is not valid" << std::endl;
      delete newConfig;
    }
    else
      camera.setConfig(newConfig);
  }
  
  if(s.camera_configs.some()) {
    const std::vector<camera_config> &configs = s.camera_configs.unwrap();

    // Iterate through each config
    for(const camera_config &conf : configs) {
      const std::vector<channel_config> &chConfigs = conf.channels;
      CameraConfig camConfig;
      
      // Iterate through each channel
      for(const channel_config &cc : chConfigs) {
        const CameraConfig::HsvBounds hsvs = {cc.bh, cc.bs, cc.bv, cc.th, cc.ts, cc.tv};
        camConfig.addChannel(hsvs);
      }
      
      // Save camera config
      camConfig.save(ConfigPath::pathToSysConfig(conf.config_name));
    }
  }
}

void BoydRunner::createFrameData(boyd::frame_data &fd)
{
  using namespace boyd;
  
  const int prevDataSize = fd.width * fd.height * 3;
  
  fd.format = "bgr8";
  fd.width  = camera.imageWidth();
  fd.height = camera.imageHeight();
  
  fd.ch_data.clear();
  for(int chanNum = 0; chanNum < camera.numChannels(); ++chanNum) {
    channel_data cd;
    const ObjectVector *const objs = camera.objects(chanNum);
    const unsigned long numBlobs = std::min<unsigned long>(objs->size(), maxNumBlobs);
    for(int i = 0; i < numBlobs; ++i) {
      const Object &obj = objs->at(i);
      blob b;
      b.centroidX = obj.centroidX;
      b.centroidY = obj.centroidY;
      b.bBoxX = obj.bBoxX;
      b.bBoxY = obj.bBoxY;
      b.bBoxWidth = obj.bBoxWidth;
      b.bBoxHeight = obj.bBoxHeight;
      b.confidence = obj.confidence;
      cd.blobs.push_back(b);
    }
    fd.ch_data.push_back(cd);
  }
  
  const int reqDataSize = fd.width * fd.height * 3;
  if(reqDataSize != prevDataSize)
    fd.data.resize(reqDataSize);
  for(uint32_t r = 0; r < fd.height; ++r)
    memcpy(fd.data.data() + r * fd.width * 3, camera.rawImageRow(r), fd.width * 3);
}

Camera BoydRunner::camera;
int BoydRunner::maxNumBlobs = 10;

// Constant strings for each relevant daylite topic
const std::string BoydRunner::frameTopic = "camera/frame_data";
const std::string BoydRunner::settingsTopic = "camera/settings";
const std::string BoydRunner::setSettingsTopic = "camera/set_settings";
