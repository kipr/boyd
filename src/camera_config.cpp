#include "camera_config.hpp"
#include "camera.hpp"

CameraConfig::CameraConfig()
{
  numChannels = 0;
  
  m_config.beginGroup(CAMERA_GROUP);
}

void CameraConfig::addChannel(const std::string &chName, uint8_t bh, uint8_t bs, uint8_t bv, uint8_t th, uint8_t ts, uint8_t tv)
{
  m_config.beginGroup(chName);
  m_config.setValue("bh", bh);
  m_config.setValue("bs", bs);
  m_config.setValue("bv", bv);
  m_config.setValue("th", th);
  m_config.setValue("ts", ts);
  m_config.setValue("tv", tv);
  m_config.setValue("type", "hsv");
  m_config.endGroup();
  
  ++numChannels;
}

bool CameraConfig::save(const std::string &path)
{
  // Put number of channels
  m_config.setValue(CAMERA_CHANNEL_NUM_KEY, numChannels);
  
  // Save
  return m_config.save(path);
}