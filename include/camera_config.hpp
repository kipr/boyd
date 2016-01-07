#ifndef _CAMERA_CONFIG_HPP_
#define _CAMERA_CONFIG_HPP_

#include "config.hpp"

class CameraConfig {
public:
  struct HsvBounds {
    uint8_t bh;
    uint8_t bs;
    uint8_t bv;
    uint8_t th;
    uint8_t ts;
    uint8_t tv;
  };
  
  CameraConfig();
  CameraConfig(const std::string &path);
  
  bool save(const std::string &path);
  
  void addChannel(const std::string &chName, HsvBounds hsvs);
  HsvBounds channel(const int channelNum);
  const std::string channelName(const int channelNum) const;
  
  int numChannels() const;
  
private:
  Config *m_config;
};

#endif