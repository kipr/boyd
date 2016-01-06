#include "boyd_runner.hpp"
#include "object.hpp"
#include "config.hpp"
#include <battlecreek/channel_data.hpp>
#include <battlecreek/blob.hpp>
#include <battlecreek/settings.hpp>

#include <daylite/node.hpp>
#include <daylite/spinner.hpp>
#include <daylite/util.hpp>

#include <iostream>
#include <thread>

using namespace daylite;
using namespace std::chrono;

void BoydRunner::run()
{
  // Load default config file
  Config *defaultConfig = Config::load(ConfigPath::defaultConfigPath());
  if(!defaultConfig) {
    std::cerr << "Failed to load the default config file" << std::endl;
    return;
  }
  camera.setConfig(*defaultConfig);
    
  // Setup daylite node, pubs, and subs
  std::shared_ptr<node> node = node::create_node("boyd");
  auto res = node->start("127.0.0.1", 8374);
  if(!res) {
    std::cerr << "Failed to join daylite!" << std::endl;
    return;
  }
  
  auto framePub = node->advertise(BoydRunner::frameTopic);
  framePub->set_firehose(true);
  auto settingsSub = node->subscribe(BoydRunner::settingsTopic, BoydRunner::receivedSettings);
  
  // Initial camera setup
  if(!camera.open(0)) {
    std::cerr << "Failed to open camera!" << std::endl;
    return;
  }
  camera.setWidth(320);
  camera.setHeight(240);
  
  int frameNum = 0;
  auto startTime = std::chrono::steady_clock::now();
  
  while(!daylite::should_exit()) {
    daylite::spinner::spin_once();
    
    // Only process/send frames if someone cares about them
    if(framePub->subscriber_count() > 0) {
      std::cout << "Num subscribers: " << framePub->subscriber_count() << std::endl;
      
      // Retreive a new frame
      camera.update();
    
      // Publish frame and blobs over daylite
      boyd::frame_data fd = BoydRunner::createFrameData();
      framePub->publish(bson(fd.bind()));
      
      // Frame timing
      ++frameNum;
      auto duration = steady_clock::now() - startTime;
      if(duration >= seconds(1)) {
        std::cout << "Published " << frameNum << " frames in " << duration_cast<milliseconds>(duration).count() << std::endl;
        frameNum = 0;
        startTime = steady_clock::now();
      }
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
  if(s.config_base_path.some())
    ConfigPath::setBasePath(s.config_base_path.unwrap());
  if(s.config_name.some()) {
    // Try to load the new config
    const Config *const newConfig = Config::load(ConfigPath::pathTo(s.config_name.unwrap()));
    if(!newConfig)
      std::cerr << "Failed to load the new config file" << std::endl;
    else
      camera.setConfig(*newConfig);
  }
}

boyd::frame_data BoydRunner::createFrameData()
{
  using namespace boyd;
  
  frame_data ret;
  ret.format = "bgr8";
  ret.width  = camera.imageWidth();
  ret.height = camera.imageHeight();
  
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
    ret.ch_data.push_back(cd);
  }
  
  ret.data.resize(ret.width * ret.height * 3);
  for(uint32_t r = 0; r < ret.height; ++r)
    memcpy(ret.data.data() + r * ret.width * 3, camera.rawImageRow(r), ret.width * 3);
  
  return ret;
}

Camera BoydRunner::camera;
int BoydRunner::maxNumBlobs = 10;

// Constant strings for each relevant daylite topic
const std::string BoydRunner::frameTopic = "camera/frame_data";
const std::string BoydRunner::settingsTopic = "camera/settings";
