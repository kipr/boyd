#include "camera.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

#include <battlecreek/channel_data.hpp>

Camera::Camera()
  : m_capture(new cv::VideoCapture)
{
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
  if(!this->isOpen())
    return false;
  
  // Try to get a new frame
  if(!m_capture->read(m_image)) {
    m_image = cv::Mat();
    this->close();
    std::cout << "Camera disconnected" << std::endl;
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
  m_capture->set(CV_CAP_PROP_FRAME_WIDTH, width);
}

void Camera::setHeight(const int height)
{
  m_capture->set(CV_CAP_PROP_FRAME_HEIGHT, height);
}

void Camera::setConfig(const Config &config)
{
  m_config = config;
  updateChannelsFromConfig();
}

int Camera::imageWidth() const
{
  return m_image.cols;
}

int Camera::imageHeight() const
{
  return m_image.rows;
}

int Camera::numChannels() const
{
  return m_channels.size();
}

const uchar *Camera::rawImageRow(const int rowNum) const
{
  return m_image.ptr(rowNum);
}

const ObjectVector *Camera::objects(int channelNum) const
{
  if(channelNum >= m_channels.size())
    return 0;
  
  return m_channels[channelNum]->objects();
}

void Camera::updateChannelsFromConfig()
{
  auto it = m_channels.begin();
  for(; it != m_channels.end(); ++it)
    delete *it;
  m_channels.clear();
  
  // Populate channels based on current config
  // TODO: Use CameraConfig to do this
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