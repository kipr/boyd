#include "camera_config.hpp"
#include <iostream>

// Keys are used in the config files
#define CAMERA_GROUP ("camera")
#define CAMERA_CHANNEL_NUM_KEY ("num_channels")
#define CAMERA_CHANNEL_TYPE_KEY ("type")
#define CAMERA_CHANNEL_GROUP_PREFIX ("channel_")

CameraConfig::CameraConfig()
  : m_config(new Config()),
  m_isValid(true)
{
  m_config->beginGroup(CAMERA_GROUP);
}

CameraConfig::CameraConfig(const std::string &path)
  : m_config(Config::load(path))
{
  m_isValid = this->checkValidity();
  
  if(m_config) {
    m_config->clearGroup();
    m_config->beginGroup(CAMERA_GROUP);
  }
}

void CameraConfig::addChannel(const HsvBounds hsvs)
{
  const int numCh = this->numChannels();
  m_config->setValue(CAMERA_CHANNEL_NUM_KEY, numCh + 1);
  
  const std::string chName = this->channelName(numCh);
  m_config->beginGroup(chName);
  m_config->setValue("bh", hsvs.bh);
  m_config->setValue("bs", hsvs.bs);
  m_config->setValue("bv", hsvs.bv);
  m_config->setValue("th", hsvs.th);
  m_config->setValue("ts", hsvs.ts);
  m_config->setValue("tv", hsvs.tv);
  m_config->setValue(CAMERA_CHANNEL_TYPE_KEY, "hsv");
  m_config->endGroup();
}

CameraConfig::HsvBounds CameraConfig::channel(const int channelNum) const
{
  if(channelNum >= this->numChannels())
    return HsvBounds();
  
  HsvBounds ret;
  m_config->beginGroup(this->channelName(channelNum));
  ret.bh = m_config->intValue("bh");
  ret.bs = m_config->intValue("bs");
  ret.bv = m_config->intValue("bv");
  ret.th = m_config->intValue("th");
  ret.ts = m_config->intValue("ts");
  ret.tv = m_config->intValue("tv");
  m_config->endGroup();
  
  return ret;
}

const std::string CameraConfig::channelName(const int channelNum) const
{
  return CAMERA_CHANNEL_GROUP_PREFIX + std::to_string(channelNum);
}

const std::string CameraConfig::channelType(const int channelNum) const
{
  m_config->beginGroup(this->channelName(channelNum));
  const std::string type = m_config->stringValue(CAMERA_CHANNEL_TYPE_KEY);
  m_config->endGroup();
  
  return type;
}

bool CameraConfig::save(const std::string &path) const
{
  return m_config->save(path);
}

int CameraConfig::numChannels() const
{
  return m_config->intValue(CAMERA_CHANNEL_NUM_KEY);
}

bool CameraConfig::isValid() const
{
  return m_isValid;
}

bool CameraConfig::checkValidity()
{
  if(!m_config) return false;
  
  m_config->clearGroup();
  
  m_config->beginGroup(CAMERA_GROUP);
  if(!m_config->containsKey(CAMERA_CHANNEL_NUM_KEY)) return false;
  
  const int numCh = this->numChannels();
  for(int chNum = 0; chNum < numCh; ++chNum) {
    const std::string chName = this->channelName(chNum);
    m_config->beginGroup(chName);
    if(!m_config->containsKey(CAMERA_CHANNEL_TYPE_KEY)) return false;
    if(!m_config->containsKey("bh")) return false;
    if(!m_config->containsKey("bs")) return false;
    if(!m_config->containsKey("bv")) return false;
    if(!m_config->containsKey("th")) return false;
    if(!m_config->containsKey("ts")) return false;
    if(!m_config->containsKey("tv")) return false;
    m_config->endGroup();
  }
  
  return true;
}