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
  
  void setWidth(const unsigned width);
  void setHeight(const unsigned height);
  
  unsigned width() const;
  unsigned height() const;
  
  void setConfig(const Config &config);
  
  const ObjectVector *objects(int channelNum) const;
  bson_t *imageBson(bool includeFrame, bool includeBlobs) const;
  
private:
  void updateChannelsFromConfig();
  
  cv::VideoCapture *m_capture;
  cv::Mat m_image;
  
  Config m_config;
  ChannelPtrVector m_channels;
};

#endif