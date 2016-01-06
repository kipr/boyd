#ifndef _CAMERA_CONFIG_HPP_
#define _CAMERA_CONFIG_HPP_

#include "config.hpp"

class CameraConfig {
public:
  CameraConfig();
  bool save(const std::string &path);
  
  void addChannel(const std::string &chName, uint8_t bh, uint8_t bs, uint8_t bv, uint8_t th, uint8_t ts, uint8_t tv);
  
private:
  Config m_config;
  int numChannels;
};

#endif