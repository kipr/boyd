#ifndef _CHANNEL_HPP_
#define _CHANNEL_HPP_

#include "object.hpp"
#include "camera_config.hpp"
#include "channel_impl.hpp"

class Channel
{
public:
  Channel(const CameraConfig &config, const int channelNum);
  ~Channel();
  
  void invalidate();
    
#ifndef SWIG
  const ObjectVector *objects() const;
#endif
  
  void setConfig(const CameraConfig &config, const int channelNum);
  
private:
  CameraConfig m_config;
  int m_channelNum;
  mutable ObjectVector m_objects;
  ChannelImpl *m_impl;
  mutable bool m_valid;
};

typedef std::vector<Channel *> ChannelPtrVector;

#endif