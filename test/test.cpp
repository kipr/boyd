#include <iostream>
#include <chrono>
#include <thread>
#include <daylite/node.hpp>
#include <daylite/spinner.hpp>
#include <daylite/bson.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "frame_data.hpp"
#include "settings.hpp"

using namespace daylite;
using namespace std::chrono;

int frameNum = 0;
time_point<steady_clock> prevTime;

void receivedFrame(const bson &msg, void *arg);

int main()
{
  // Setup daylite node, pubs, and subs
  auto node = node::create_node("tester");
  auto res = node->start("127.0.0.1", 8374);
  if(!res) {
    std::cerr << "Failed to join daylite!" << std::endl;
    return 1;
  }
  
  auto frameSub = node->subscribe("camera/frame_data", receivedFrame);
  auto settingsPub = node->advertise("camera/settings");
  
  prevTime = steady_clock::now();
  for(;;) {
    daylite::spinner::spin_once();
    std::this_thread::sleep_for(milliseconds(50));
  }
  
  return 0;
}

void receivedFrame(const bson &msg, void *arg)
{ 
  using namespace cv;
  using namespace boyd;
  
  ++frameNum;
  //std::cout << "Received frame " << ++frameNum << std::endl;
  
  frame_data fd = frame_data::unbind(msg);
  //std::cout << "(" << unsigned(fd.data[0]) << ", " << unsigned(fd.data[1]) << ", " << unsigned(fd.data[2]) << ")" << std::endl;
  
  Mat frame = Mat::zeros(fd.height, fd.width, 16);
  int offset = 0;
  for(int i = 0; i < frame.rows; ++i)
    for(int j = 0; j < frame.cols; ++j) {
      frame.at<Vec3b>(i, j) = Vec3b(fd.data[offset], fd.data[offset + 1], fd.data[offset + 2]);
      offset += 3; 
    }
  
  if(fd.ch_data[0].blobs.size() > 0) {
    auto bigBlob = fd.ch_data[0].blobs[0];
    std::cout << "Biggest blob confidence: " << bigBlob.confidence << std::endl;
    Rect blobRect(bigBlob.bBoxX, bigBlob.bBoxY, bigBlob.bBoxX + bigBlob.bBoxWidth, bigBlob.bBoxY + bigBlob.bBoxHeight);
    rectangle(frame, blobRect, Scalar(255, 0, 0)); 
  }
  imshow("received", frame);
  waitKey(10);
  
  //std::cout << "Num channels: " << fd.ch_data.size() << std::endl;
  //std::cout << fd.ch_data[0].blobs.size() << " blobs" << std::endl;
  
  auto currTime = steady_clock::now();
  if(currTime - prevTime >= milliseconds(1000)) {
    auto duration = currTime - prevTime;
    std::cout << "Received " << frameNum << " frames in " << duration_cast<milliseconds>(duration).count() << std::endl;
    frameNum = 0;
    prevTime = currTime;
  }
}