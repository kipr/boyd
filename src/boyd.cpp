/*#include <opencv2/videoio.hpp>
#include <daylite/node.hpp>
#include <iostream>
#include <thread>
#include <chrono>

#include "frame_data.hpp"
#include "settings.hpp"
#include "camera.hpp"

using namespace daylite;
using namespace std;
using namespace std::chrono;*/

#include "boyd_runner.hpp"

int main(int argc, char *argv[])
{
  BoydRunner::run();
  /*auto n = node::create_node("boyd");
  
  auto res = n->join_daylite("127.0.0.1", 8374);
  
  if(!res)
  {
    cerr << "Failed to join daylite" << endl;
  }
  
  const auto pub = n->advertise("camera/frame_data");
  
  Camera camera;
  Config *defaultConfig = Config::load(ConfigPath::defaultConfigPath());
  if(!defaultConfig)
    std::cerr << "Failed to load the default config file" << std::endl;
  else
    camera.setConfig(*defaultConfig);
  for(;;)
  {    
    if(camera.open(0))
      break;
    
    std::cout << "Failed to open capture! Retrying in 1 second..." << std::endl;
    this_thread::sleep_for(seconds(1));
  }
  
  const int camWidth = 480;
  const int camHeight = 320;
  camera.setWidth(camWidth);
  camera.setHeight(camHeight);
  
  const uint32_t fps = 12;
  int frame_num = 0;
  auto start_time = chrono::system_clock::now();
  
  //auto small_start = chrono::steady_clock::now();
  //auto small_end = chrono::steady_clock::now();
  //auto small_diff = small_end - small_start;
  
  for(;;)
  {
    auto curr_time = chrono::system_clock::now();
    auto diff = curr_time - start_time;
    if(diff > chrono::seconds(1))
    {
      //auto end_time = chrono::system_clock::now();
      //auto diff = end_time - start_time;
      std::cout << "Drew " << frame_num << " frames in " << chrono::duration<double, milli>(diff).count() << " ms" << std::endl;
      frame_num = 0;
      start_time = chrono::system_clock::now();
    }
    ++frame_num;
    
    //small_start = chrono::steady_clock::now();
    camera.update();

    //small_end = chrono::steady_clock::now();
    //small_diff = small_end - small_start;
    //std::cout << "Time to capture: " << chrono::duration<double, milli>(small_diff).count() << " ms" << std::endl;
    
    pub->publish(camera.imageBson());
    
    //this_thread::sleep_for(milliseconds(static_cast<uint32_t>(1000.0 / fps)));
  }*/
}