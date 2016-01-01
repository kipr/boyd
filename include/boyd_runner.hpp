#ifndef _BOYD_RUNNER_HPP_
#define _BOYD_RUNNER_HPP_

#include <memory>
#include <bson.h>
#include "camera.hpp"

using namespace std::chrono;

class BoydRunner
{
public:
  static void run();
  
private:
  BoydRunner();
  static void receivedSettings(const bson_t *msg, void *);
  
  static Camera camera;
  
  static const std::string frameTopic;
  static const std::string settingsTopic;
};

#endif