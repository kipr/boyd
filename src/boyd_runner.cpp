#include "boyd_runner.hpp"
#include "settings.hpp"

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
  
  // Initial camera setup
  if(!camera.open(0)) {
    std::cerr << "Failed to open camera!" << std::endl;
    return;
  }
  camera.setWidth(240);
  camera.setHeight(180);
  
  // Run forever
  for(;;) {
    daylite::spinner::spin_once();
    
    // Only process/send frames if someone cares about them
    if(framePub->subscriber_count() > 0) {
      std::cout << "Num subscribers: " << framePub->subscriber_count() << std::endl;
      
      // Retreive a new frame
      camera.update();
    
      // Publish frame and blobs over daylite
      boyd::frame_data fd = camera.createFrameData();
      framePub->publish(fd.bind());
    }
    
    // TODO: This is an arbitrary sleep interval for testing purposes
    std::this_thread::sleep_for(milliseconds(20));
  }
}

// Called when we receive setting changes
void BoydRunner::receivedSettings(const bson_t *msg, void *)
{
  using namespace boyd;
  
  std::cout << "Received settings" << std::endl;
  settings s = settings::unbind(msg);
  if(s.width.some())
    camera.setWidth(s.width.unwrap());
  if(s.height.some())
    camera.setHeight(s.height.unwrap());
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

Camera BoydRunner::camera;

// Constant strings for each relevant daylite topic
const std::string BoydRunner::frameTopic = "camera/frame_data";
const std::string BoydRunner::settingsTopic = "camera/settings";