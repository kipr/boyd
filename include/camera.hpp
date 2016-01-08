#ifndef _CAMERA_HPP_
#define _CAMERA_HPP_

#include <opencv2/highgui/highgui.hpp>
#include "channel.hpp"
#include "camera_config.hpp"

class Camera
{
public:
  Camera();
  ~Camera();
  
  bool open(const int number = 0);
  bool isOpen() const;
  bool close();
  bool update();
  
  void setWidth(const int width);
  void setHeight(const int height);
  void setConfig(const CameraConfig *config);
  
  int imageWidth() const;
  int imageHeight() const;
  int numChannels() const;
  const uchar *rawImageRow(const int rowNum) const;
  const ObjectVector *objects(const int channelNum) const;
  
private:
  void updateChannelsFromConfig();
  
  cv::VideoCapture *m_capture;
  cv::Mat m_image;
  
  const CameraConfig *m_config;
  ChannelPtrVector m_channels;
};

#endif