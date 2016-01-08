#include "camera_config.hpp"
#include "camera.hpp"

CameraConfig::CameraConfig()
  : m_config(new Config())
{
  m_config->beginGroup(CAMERA_GROUP);
}

CameraConfig::CameraConfig(const std::string &path)
  : m_config(Config::load(path))
{
  m_config->beginGroup(CAMERA_GROUP);
}

bool CameraConfig::save(const std::string &path)
{
  return m_config->save(path);
}

void CameraConfig::addChannel(const std::string &chName, HsvBounds hsvs)
{
  const bool existed = m_config->containsKey(chName);
  
  m_config->beginGroup(chName);
  m_config->setValue("bh", hsvs.bh);
  m_config->setValue("bs", hsvs.bs);
  m_config->setValue("bv", hsvs.bv);
  m_config->setValue("th", hsvs.th);
  m_config->setValue("ts", hsvs.ts);
  m_config->setValue("tv", hsvs.tv);
  m_config->setValue("type", "hsv");
  m_config->endGroup();
  
  // Is this a new channel?
  if(!existed) {
    const int numChannels = m_config->intValue(CAMERA_CHANNEL_NUM_KEY);
    m_config->setValue(CAMERA_CHANNEL_NUM_KEY, numChannels + 1);
  }
}

CameraConfig::HsvBounds CameraConfig::channel(const int channelNum)
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

int CameraConfig::numChannels() const
{
  return m_config->intValue(CAMERA_CHANNEL_NUM_KEY);
}