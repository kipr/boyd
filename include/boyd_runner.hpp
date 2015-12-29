#ifndef _BOYD_RUNNER_HPP_
#define _BOYD_RUNNER_HPP_

#include <memory>
#include "camera.hpp"

using namespace std::chrono;

class BoydRunner
{
public:
  static void run();
  
private:
  BoydRunner();
  static void receivedSettings(const bson_t *msg, void *);
  static void receivedBeat(const bson_t *msg, void *);
  
  static Camera camera;
  
  static time_point<steady_clock> lastFramesBeat;
  static time_point<steady_clock> lastBlobsBeat;
  static bool framesActive;
  static bool blobsActive;
  
  static const std::string frameTopic;
  static const std::string settingsTopic;
  static const std::string beatTopic;
};

#endif