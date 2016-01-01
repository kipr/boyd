#ifndef _CAMERA_HPP_
#define _CAMERA_HPP_

#include <opencv2/videoio.hpp>
#include "frame_data.hpp"
#include "channel.hpp"
#include "config.hpp"

// These keys are used in the config files
#define CAMERA_GROUP ("camera")
#define CAMERA_CHANNEL_NUM_KEY ("num_channels")
#define CAMERA_CHANNEL_TYPE_KEY ("type")
#define CAMERA_CHANNEL_GROUP_PREFIX ("channel_")

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
  
  int width() const;
  int height() const;
  
  void setConfig(const Config &config);
  
  const ObjectVector *objects(int channelNum) const;
  boyd::frame_data createFrameData() const;
  
private:
  void updateChannelsFromConfig();
  
  cv::VideoCapture *m_capture;
  cv::Mat m_image;
  
  Config m_config;
  ChannelPtrVector m_channels;
  
  int m_width;
  int m_height;
};

#endif