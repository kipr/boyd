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
  
  void addChannel(const HsvBounds hsvs);
  HsvBounds channel(const int channelNum) const;
  const std::string channelName(const int channelNum) const;
  const std::string channelType(const int channelNum) const;
  
  bool save(const std::string &path) const;
  
  int numChannels() const;
  bool isValid() const;
  
private:
  bool checkValidity();
  
  Config *m_config;
  bool m_isValid;
};

#endif