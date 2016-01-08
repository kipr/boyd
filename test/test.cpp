#include <iostream>
#include <chrono>
#include <thread>
#include <daylite/node.hpp>
#include <daylite/spinner.hpp>
#include <daylite/bson.hpp>
#include <daylite/util.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <battlecreek/frame_data.hpp>
#include <battlecreek/settings.hpp>

using namespace daylite;
using namespace std::chrono;

int frameNum = 0;
time_point<steady_clock> prevTime;

void receivedFrame(const bson &msg, void *arg);
void onMouseEvent(int event, int x, int y, int f, void *data);

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
  auto settingsPub = node->advertise("camera/set_settings");
  
  prevTime = steady_clock::now();
  while(!daylite::should_exit()) {
    daylite::spinner::spin_once();
    std::this_thread::sleep_for(milliseconds(1000));
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
  std::cout << "Received frame " << fd.frameNum << std::endl;
  
  Mat frame = Mat::zeros(fd.height, fd.width, 16);
  int offset = 0;
  for(int i = 0; i < frame.rows; ++i)
    for(int j = 0; j < frame.cols; ++j) {
      frame.at<Vec3b>(i, j) = Vec3b(fd.data[offset], fd.data[offset + 1], fd.data[offset + 2]);
      offset += 3; 
    }
  
  if(fd.ch_data[0].blobs.size() > 0) {
    const auto bigBlob = fd.ch_data[0].blobs[0];
    std::cout << "Biggest blob confidence: " << bigBlob.confidence << std::endl;
    const Rect blobRect(bigBlob.bBoxX, bigBlob.bBoxY, bigBlob.bBoxWidth, bigBlob.bBoxHeight);
    rectangle(frame, blobRect, Scalar(255, 0, 0));
  }
  imshow("received", frame);
  cv::setMouseCallback("received", onMouseEvent, &frame);
  waitKey(10);
  
  auto currTime = steady_clock::now();
  if(currTime - prevTime >= milliseconds(1000)) {
    auto duration = currTime - prevTime;
    std::cout << "Received " << frameNum << " frames in " << duration_cast<milliseconds>(duration).count() << std::endl;
    frameNum = 0;
    prevTime = currTime;
  }
}

void onMouseEvent(int event, int x, int y, int f, void *data) {
  using namespace cv;
  
  if(event != EVENT_LBUTTONDOWN)
    return;
  
  Mat inputImage = *((Mat *)data);
  
  const Vec3b rgb = inputImage.at<Vec3b>(y, x);
  
  Mat HSV;
  Mat RGB = inputImage(Rect(x, y, 1, 1));
  cvtColor(RGB, HSV, CV_BGR2HSV);
  
  const Vec3b hsv = HSV.at<Vec3b>(0, 0);
  
  std::cout << "(H, S, V) = (" << unsigned(hsv.val[0]) << ", " << unsigned(hsv.val[1]) << ", " << unsigned(hsv.val[2]) << ")" << std::endl;
}