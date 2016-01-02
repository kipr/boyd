#ifndef _BOYD_RUNNER_HPP_
#define _BOYD_RUNNER_HPP_

#include <daylite/bson.hpp>
#include "camera.hpp"
#include "frame_data.hpp"

class BoydRunner
{
public:
  static void run();
  
private:
  BoydRunner();
  static void receivedSettings(const daylite::bson &msg, void *);
  static boyd::frame_data createFrameData();
  
  static Camera camera;
  
  static int maxNumBlobs;
  
  static const std::string frameTopic;
  static const std::string settingsTopic;
};

#endif