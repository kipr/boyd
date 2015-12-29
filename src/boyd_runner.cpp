#include "boyd_runner.hpp"
#include "settings.hpp"
#include "heartbeat.hpp"

#include <daylite/node.hpp>
#include <daylite/spinner.hpp>

#include <iostream>
#include <thread>
#include <chrono>

using namespace std::chrono;
using namespace std::placeholders;
using namespace daylite;

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
  auto settingsSub = node->subscribe(BoydRunner::settingsTopic, BoydRunner::receivedSettings);
  auto beatSub  = node->subscribe(BoydRunner::beatTopic, &BoydRunner::receivedBeat);
  
  // Setup camera
  if(!camera.open(0)) {
    std::cerr << "Failed to open camera!" << std::endl;
    return;
  }
  camera.setWidth(240);
  camera.setHeight(180);
    
  for(;;) {
    // Spin daylite
    daylite::spinner::spin_once();
    
    // If not active, just sleep
    if(!framesActive && !blobsActive) {
      std::this_thread::sleep_for(milliseconds(20));
      continue;
    }
    
    // If it has been 5 seconds since the last heartbeat, deactivate
    auto currTime = steady_clock::now();
    if(currTime - lastFramesBeat > seconds(5))
      framesActive = false;
    if(currTime - lastBlobsBeat > seconds(5))
      blobsActive = false;
    if(!framesActive && !blobsActive) {
      std::this_thread::sleep_for(milliseconds(20));
      continue;
    }
    
    // Update camera
    camera.update();
    
    // Publish frame and blobs
    const bson_t *const imBson = camera.imageBson(framesActive, blobsActive);
    framePub->publish(imBson);
    
    // Sleep
    std::this_thread::sleep_for(milliseconds(20));
  }
}

void BoydRunner::receivedSettings(const bson_t *msg, void *)
{
  using namespace bson_bind;
  
  std::cout << "Received settings" << std::endl;
  settings s = settings::unbind(msg);
  if(s.width.some())
    camera.setWidth(s.width.unwrap());
  if(s.height.some())
    camera.setHeight(s.height.unwrap());
  if(s.config_base_path.some())
    ConfigPath::setBasePath(s.config_base_path.unwrap());
  if(s.config_name.some()) {
    const Config *const newConfig = Config::load(ConfigPath::pathTo(s.config_name.unwrap()));
    if(!newConfig)
      std::cerr << "Failed to load the new config file" << std::endl;
    else
      camera.setConfig(*newConfig);
  }
}

void BoydRunner::receivedBeat(const bson_t *msg, void *)
{
  using namespace bson_bind;
  
  std::cout << "Received beat" << std::endl;
  
  // Analyze message to figure out if we should send frames, blobs, or both
  auto currTime = steady_clock::now();
  heartbeat beat = heartbeat::unbind(msg);
  if(beat.frames) {
    BoydRunner::framesActive = true;
    BoydRunner::lastFramesBeat = currTime;
  }
  if(beat.blobs) {
    BoydRunner::blobsActive = true;
    BoydRunner::lastBlobsBeat = currTime;
  }
}

Camera BoydRunner::camera;

time_point<steady_clock> BoydRunner::lastFramesBeat;
time_point<steady_clock> BoydRunner::lastBlobsBeat;
bool BoydRunner::framesActive = false;
bool BoydRunner::blobsActive = false;

const std::string BoydRunner::frameTopic = "camera/frame_data";
const std::string BoydRunner::settingsTopic = "camera/settings";
const std::string BoydRunner::beatTopic = "camera/heartbeat";