#include "channel.hpp"
#include "frame_data.hpp"
#include <iostream>

Channel::Channel(const Config &config)
  : m_config(config),
  m_impl(0),
  m_valid(false)
{
  m_objects.clear();
  const std::string type = config.stringValue("type");
  if(type.empty()) {
    std::cerr << "No type specified in config." << std::endl;
    return;
  }
  
  m_impl = ChannelImplManager::channelImpl(type);
  if(!m_impl) {
    std::cerr << "Type " << type << " not found" << std::endl;
    return;
  }
}

Channel::~Channel()
{
}

void Channel::invalidate()
{
  m_valid = false;
}

struct AreaComparator
{
public:
  bool operator()(const Object &left, const Object &right) {
    return left.bBoxArea > right.bBoxArea;
  }
} LargestAreaFirst;

const ObjectVector *Channel::objects() const
{
  if(!m_impl)
    return 0;
  if(!m_valid) {
    m_objects.clear();
    m_objects = m_impl->objects(m_config);
    std::sort(m_objects.begin(), m_objects.end(), LargestAreaFirst);
    m_valid = true;
  }
  
  return &m_objects;
}

void Channel::setConfig(const Config &config)
{
  m_config = config;
  m_valid = false;
}