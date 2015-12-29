#ifndef _CHANNEL_HPP_
#define _CHANNEL_HPP_

#include "object.hpp"
#include "config.hpp"
#include "channel_impl.hpp"

class Channel
{
public:
  Channel(const Config &config);
  ~Channel();
  
  void invalidate();
    
#ifndef SWIG
  const ObjectVector *objects() const;
#endif
  
  void setConfig(const Config &config);
  
private:
  Config m_config;
  mutable ObjectVector m_objects;
  ChannelImpl *m_impl;
  mutable bool m_valid;
};

typedef std::vector<Channel *> ChannelPtrVector;

#endif