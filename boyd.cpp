#include <opencv2/videoio.hpp>
#include <daylite/node.hpp>
#include <iostream>
#include <thread>
#include <chrono>

#include "image_color.hpp"
#include "settings.hpp"

using namespace daylite;
using namespace cv;
using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[])
{
  auto n = node::create_node("boyd");
  
  auto res = n->join_daylite("127.0.0.1", 8374);
  
  if(!res)
  {
    cerr << "Failed to join daylite" << endl;
  }
  
  VideoCapture cap;
  
  const auto pub = n->advertise("camera/image_color");
  
  uint32_t fps = 1;
  for(;;)
  {
    this_thread::sleep_for(milliseconds(static_cast<uint32_t>(1000.0 / fps)));
    if(!cap.isOpened())
    {
      if(!cap.open(0)) continue;
    }
    
    Mat m;
    cap >> m;
    
    bson_bind::image_color p;
    p.format = "bgr8";
    p.width  = cap.get(CAP_PROP_FRAME_WIDTH);
    p.height = cap.get(CAP_PROP_FRAME_HEIGHT);
    
    p.data.resize(p.width * p.height * 3);
    for(uint32_t r = 0; r < p.height; ++r)
    {
      memcpy(p.data.data() + r * p.width, m.ptr(r), p.width);
    }
    
    pub->publish(p.bind());
  }
}