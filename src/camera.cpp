#include "camera.hpp"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

Camera::Camera()
  : m_capture(new cv::VideoCapture)
{ 
  setWidth(160);
  setHeight(120);
}

Camera::~Camera()
{
  auto it = m_channels.begin();
  for(; it != m_channels.end(); ++it)
    delete *it;
  
  delete m_capture;
}

bool Camera::open(const int number)
{
  if(!m_capture || m_capture->isOpened())
    return false;
  
  return m_capture->open(number);
}

bool Camera::isOpen() const
{
  return m_capture && m_capture->isOpened();
}

bool Camera::close()
{
  if(!m_capture || !m_capture->isOpened())
    return false;
  
  m_capture->release();
  
  return true;
}

bool Camera::update()
{
  if(!m_capture->read(m_image)) {
    m_image = cv::Mat();
    return false;
  }
  
  // No need to update channels if there are none
  if(!m_channels.empty()) {
    // Dirty all channel impls
    ChannelImplManager::setImage(m_image);
  
    // Invalidate all channels
    auto it = m_channels.begin();
    for(; it != m_channels.end(); ++it)
      (*it)->invalidate();
  }
    
  return true;
}

void Camera::setWidth(const int width)
{
  m_capture->set(cv::CAP_PROP_FRAME_WIDTH, width);
  m_width = width;
}

void Camera::setHeight(const int height)
{
  m_capture->set(cv::CAP_PROP_FRAME_HEIGHT, height);
  m_height = height;
}

int Camera::width() const
{
  return m_width;
}

int Camera::height() const
{
  return m_height;
}

void Camera::setConfig(const Config &config)
{
  m_config = config;
  updateChannelsFromConfig();
}

const ObjectVector *Camera::objects(int channelNum) const
{
  if(channelNum >= m_channels.size())
    return 0;
  
  return m_channels[channelNum]->objects();
}

bson_t *Camera::imageBson() const
{
  bson_bind::frame_data fd;
  fd.format = "bgr8";
  fd.width  = this->width();
  fd.height = this->height();
  
  const ObjectVector *const objs = this->objects(0);
  std::cout << "Found " << objs->size() << " blobs" << std::endl;
  for(Object obj : *objs) {
    bson_bind::blob b;
    b.centroidX = obj.centroidX;
    b.centroidY = obj.centroidY;
    b.bBoxX = obj.bBoxX;
    b.bBoxY = obj.bBoxY;
    b.bBoxWidth = obj.bBoxWidth;
    b.bBoxHeight = obj.bBoxHeight;
    b.confidence = obj.confidence;
    fd.blobs.push_back(b);
  }
  
  fd.data.resize(fd.width * fd.height * 3);
  for(uint32_t r = 0; r < fd.height; ++r)
    memcpy(fd.data.data() + r * fd.width * 3, m_image.ptr(r), fd.width * 3);
  
  return fd.bind();
}

void Camera::updateChannelsFromConfig()
{
  auto it = m_channels.begin();
  for(; it != m_channels.end(); ++it)
    delete *it;
  m_channels.clear();
  
  m_config.clearGroup();
  m_config.beginGroup(CAMERA_GROUP);
  const int numChannels = m_config.intValue(CAMERA_CHANNEL_NUM_KEY);
  if(numChannels <= 0)
    return;
  for(int i  = 0; i < numChannels; ++i) {
    std::stringstream stream;
    stream << CAMERA_CHANNEL_GROUP_PREFIX;
    stream << i;
    m_config.beginGroup(stream.str());
    m_channels.push_back(new Channel(m_config));
    m_config.endGroup();
  }
  m_config.endGroup();
}