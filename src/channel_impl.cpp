#include "channel_impl.hpp"

#include <opencv2/imgproc/imgproc.hpp>

// ChannelImpl

ChannelImpl::ChannelImpl()
  : m_dirty(true)
{
}

ChannelImpl::~ChannelImpl()
{
}

void ChannelImpl::setImage(const cv::Mat &image)
{
  if(image.empty())
    m_image = cv::Mat();
  else
    m_image = image;
  
  m_dirty = true;
}

ObjectVector ChannelImpl::objects(const CameraConfig &config, const int channelNum)
{
  if(m_dirty) {
    update(m_image);
    m_dirty = false;
  }
  
  return findObjects(config, channelNum);
}

// ChannelImplManager

// TODO: Add qr channel impl to map
std::map<std::string, ChannelImpl *> ChannelImplManager::m_channelImpls = {{"hsv", new HsvChannelImpl()}};

void ChannelImplManager::setImage(const cv::Mat &image)
{
	std::map<std::string, ChannelImpl *>::iterator it = m_channelImpls.begin();
	for(; it != m_channelImpls.end(); ++it) it->second->setImage(image);
}

ChannelImpl *ChannelImplManager::channelImpl(const std::string &name)
{
	std::map<std::string, ChannelImpl *>::iterator it = m_channelImpls.find(name);
	return (it == m_channelImpls.end()) ? 0 : it->second;
}

// HsvChannelImpl

HsvChannelImpl::HsvChannelImpl()
{
}

void HsvChannelImpl::update(const cv::Mat &image)
{
  if(image.empty()) {
    m_image = cv::Mat();
    return;
  }
  
  cv::cvtColor(image, m_image, cv::COLOR_BGR2HSV);
}

ObjectVector HsvChannelImpl::findObjects(const CameraConfig &config, const int channelNum)
{
  if(m_image.empty())
    return ObjectVector();
  
  const CameraConfig::HsvBounds hsvs = config.channel(channelNum);
  cv::Vec3b top(hsvs.th, hsvs.ts, hsvs.tv);
  cv::Vec3b bottom(hsvs.bh, hsvs.bs, hsvs.bv);
  
  cv::Mat fixed = m_image;
  if(bottom[0] > top[0]) {
    // Modulo 180
    // TODO: Optimize for ARM?
    const uchar adjH = 180 - bottom[0];
    for(int i = 0; i < fixed.rows; ++i) {
      uchar *row = fixed.ptr<uchar>(i);
      for(int j = 0; j < fixed.cols; ++j) {
        row[j * fixed.elemSize()] += adjH;
        row[j * fixed.elemSize()] %= 180;
      }
    }
    
    cv::Vec3b adj(adjH, 0, 0);
    bottom = cv::Vec3b(0, bottom[1], bottom[2]);
    cv::add(adj, top, top);
  }
  
  cv::Mat only;
  cv::inRange(fixed, bottom, top, only);
  
  std::vector<std::vector<cv::Point> > c;
  cv::findContours(only, c, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_TC89_L1);
  
  std::vector<cv::Moments> m(c.size());
  for(std::vector<cv::Moments>::size_type i = 0; i < c.size(); ++i) {
    m[i] = cv::moments(c[i], false);
  }
  
  ObjectVector ret;
  for(ObjectVector::size_type i = 0; i < c.size(); ++i) {
    const cv::Rect rect = cv::boundingRect(c[i]);
    if(rect.width < 4 && rect.height < 4) continue;
    
    const int confidence = m[i].m00 * 100 / rect.area();
    ret.push_back(Object(m[i].m10 / m[i].m00, m[i].m01 / m[i].m00,
      rect.x, rect.y, rect.width, rect.height, confidence));
  }
  
  return ret;
}