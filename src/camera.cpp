#include "camera.hpp"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

#include "channel_data.hpp"

Camera::Camera()
  : m_capture(new cv::VideoCapture)
{
  // Default to 160x120 images
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
  // Get a new frame
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

boyd::frame_data Camera::createFrameData(const int maxNumBlobs) const
{
  using namespace boyd;
  
  frame_data ret;
  ret.format = "bgr8";
  ret.width  = this->width();
  ret.height = this->height();
  
  for(int chanNum = 0; chanNum < m_channels.size(); ++chanNum) {
    const ObjectVector *const objs = this->objects(chanNum);
    std::cout << "Channel " << chanNum << ": " << objs->size() << " blobs" << std::endl;
    channel_data cd;
    const unsigned long numBlobs = std::min<unsigned long>(objs->size(), maxNumBlobs);
    for(int i = 0; i < numBlobs; ++i) {
      const Object &obj = objs->at(i);
      blob b;
      b.centroidX = obj.centroidX;
      b.centroidY = obj.centroidY;
      b.bBoxX = obj.bBoxX;
      b.bBoxY = obj.bBoxY;
      b.bBoxWidth = obj.bBoxWidth;
      b.bBoxHeight = obj.bBoxHeight;
      b.confidence = obj.confidence;
      cd.blobs.push_back(b);
    }
    ret.ch_data.push_back(cd);
  }
  
  ret.data.resize(ret.width * ret.height * 3);
  for(uint32_t r = 0; r < ret.height; ++r)
    memcpy(ret.data.data() + r * ret.width * 3, m_image.ptr(r), ret.width * 3);
  
  return ret;
}

void Camera::updateChannelsFromConfig()
{
  auto it = m_channels.begin();
  for(; it != m_channels.end(); ++it)
    delete *it;
  m_channels.clear();
  
  // Populate channels based on current config
  m_config.clearGroup();
  m_config.beginGroup(CAMERA_GROUP);
  const int numChannels = m_config.intValue(CAMERA_CHANNEL_NUM_KEY);
  const int numSupported = std::min(numChannels, 4); // Max 4 channels
  for(int i  = 0; i < numSupported; ++i) {
    std::stringstream stream;
    stream << CAMERA_CHANNEL_GROUP_PREFIX;
    stream << i;
    m_config.beginGroup(stream.str());
    m_channels.push_back(new Channel(m_config));
    m_config.endGroup();
  }
  m_config.endGroup();
}