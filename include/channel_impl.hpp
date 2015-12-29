#ifndef _CHANNEL_IMPL_HPP_
#define _CHANNEL_IMPL_HPP_

#include <opencv2/core.hpp>
//#include <zbar.h>

#include "object.hpp"
#include "config.hpp"

class ChannelImpl
{
public:
  ChannelImpl();
  virtual ~ChannelImpl();
  
  void setImage(const cv::Mat &image);
  ObjectVector objects(const Config &config);
  
protected:
  virtual void update(const cv::Mat &image) = 0;
  virtual ObjectVector findObjects(const Config &config) = 0;
  
private:
  bool m_dirty;
  cv::Mat m_image;
};

class ChannelImplManager
{
public:
  //ChannelImplManager();
  //~ChannelImplManager();
  
  static void setImage(const cv::Mat &image);
  static ChannelImpl *channelImpl(const std::string &name);
  
private:
  static std::map<std::string, ChannelImpl *> m_channelImpls;
};

class HsvChannelImpl : public ChannelImpl
{
public:
  HsvChannelImpl();
  virtual void update(const cv::Mat &image);
  virtual ObjectVector findObjects(const Config &config);
  
private:
  cv::Mat m_image;
};

/*class BarcodeChannelImpl : public ChannelImpl
{
public:
  BarcodeChannelImpl();
  virtual void update(const cv::Mat &image);
  virtual ObjectVector findObjects(const Config &config);
  
private:
  cv::Mat m_gray;
  zbar::Image m_image;
  zbar::ImageScanner m_scanner;
};*/

#endif