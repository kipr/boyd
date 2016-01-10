#include "camera.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

#include <battlecreek/channel_data.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

using namespace std;

class v4l
{
public:
  v4l(const std::string &device)
    : _device(device)
    , _fd(-1)
  {
  }
  
  ~v4l()
  {
    close();
  }
  
  bool open()
  {
    struct stat st;
    if (-1 == stat(_device.c_str(), &st))
    {
      fprintf(stderr, "Cannot identify '%s': %d, %s\n", _device.c_str(), errno, strerror(errno));
      return false;
    }

    if (!S_ISCHR(st.st_mode))
    {
      fprintf(stderr, "%s is no device\n", _device.c_str());
      return false;
    }

    int fd = open(_device.c_str(), O_RDWR | O_NONBLOCK, 0);
    if(fd < 0)
    {
      fprintf(stderr, "Cannot open '%s': %d, %s\n", _device.c_str(), errno, strerror(errno));
      return false;
    }
    
    _fd = fd;
    
    if(!init_device()) return false;
    start_capturing();
    return true;
  }
  
  bool is_open() const { return _fd >= 0; }
  
  void next(cv::Mat &mat)
  {
    for (;;)
    {
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(fd, &fds);

      timeval tv;
      tv.tv_sec = 2;
      tv.tv_usec = 0;

      int r = select(fd + 1, &fds, NULL, NULL, &tv);

      if (-1 == r)
      {
        if (EINTR == errno) continue;
        cerr << "select failed (" << errno << ")" << endl;
      }

      if (0 == r)
      {
        cerr << "select timeout" << endl;
      }

      if (read_frame(mat)) break;
    }
  }
  
  bool close()
  {
    if(_fd < 0) return false;
    uninit_device();
    ::close(_fd);
  }
  
private:
  std::string _device;
  int _fd;
  
  struct buffer
  {
    buffer() : start(0), length(0U) {}
    void *start;
    size_t length;
  };
  
  int xioctl(int fh, int request, void *arg)
  {
    int r = 0;
    do
    {
      r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);
    return r;
  }
  
  bool read_frame(cv::Mat &mat)
  {
    struct v4l2_buffer buf;
    unsigned int i = 0;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (-1 == xioctl(_fd, VIDIOC_DQBUF, &buf))
    {
      switch (errno)
      {
      case EAGAIN: return false;
      case EIO:
      default: cerr << "VIDIOC_DQBUF (" << errno << ")" << endl;
      }
    }

    assert(buf.index < _buffers.size());
    mat = cv::Mat(240, 320, CV_8UC3, _buffers[buf.index].start);
    if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
    {
      cerr << "VIDIOC_QBUF (" << errno << ")" << endl;
      return false;
    }

    return true;
  }
  
  void start_capturing()
  {
    enum v4l2_buf_type type;
    for (unsigned i = 0; i < _buffers.size(); ++i)
    {
      struct v4l2_buffer buf;
      memset(&buf, 0, sizeof(buf));
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      buf.index = i;
      if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
      {
        cerr << "VIDIOC_QBUF (" << errno << ")" << endl;
      }
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
    {
      cerr << "VIDIOC_STREAMON (" << errno << ")" << endl;
    }
  }
  
  void uninit_device(void)
  {
    for (unsigned i = 0; i < n_buffers; ++i)
    {
      if (-1 == munmap(buffers[i].start, buffers[i].length))
      {
        cerr << "munmap failed (" << errno << ")" << endl;
      }
    }
    free(buffers);
  }
  
  bool init_device()
  {
    struct v4l2_capability cap;
    if (xioctl(fd, VIDIOC_QUERYCAP, &cap) < 0)
    {
      if (EINVAL == errno) {
        cerr << _device << " is no V4L2 device" << endl;
        return false;
      }
      else
      {
        cerr << "VIDIOC_QUERYCAP (" << errno << ")" << endl;
        return false;
      }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
      cerr << _device << " is no video capture device" << endl;
      return false;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING))
    {
      cerr << _device << " does not support streaming i/o" << endl;
      return false;
    }

    /* Select video input, video standard and tune here. */
    struct v4l2_cropcap cropcap;
    memset(&cropcap, 0, sizeof(cropcap));
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap))
    {
      struct v4l2_crop crop;
      crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      crop.c = cropcap.defrect;

      if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop))
      {
        switch (errno)
        {
        case EINVAL: break;
        default: break;
        }
      }
    }

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = 320;
    fmt.fmt.pix.height      = 240;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

    if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)) errno_exit("VIDIOC_S_FMT");

    unsigned min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min) fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min) fmt.fmt.pix.sizeimage = min;
    return init_mmap();
  }
  
  bool init_mmap(void)
  {
    struct v4l2_requestbuffers req;

    memset(&req, 0, sizeof(req));

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
  
    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
    {
      if (EINVAL == errno)
      {
        cerr << _device << " does not support memory mapping" << endl;
        return false;
      }
      else
      {
        cerr << "VIDIOC_REQBUFS (" << errno << ")" << endl;
        return false;
      }
    }
  
    if (req.count < 2)
    {
      cerr << "Insufficient buffer memory on " << _device << endl;
      return false;
    }

    _buffers.resize(req.count);
    size_t i = 0;
    for (auto it = _buffers.begin(); it != _buffers.end(); ++it, ++i)
    {
      struct v4l2_buffer buf;
      memset(&buf, 0, sizeof(buf));

      buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      buf.index  = i;

      if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf)
      {
        cerr << "VIDIOC_QUERYBUF (" << errno << ")" << endl;
      }
    
      it->length = buf.length;
      it->start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
        MAP_SHARED, _fd, buf.m.offset);

      if (MAP_FAILED == it->start)
      {
        cerr << "Map failed (" << errno << ")" << endl;
      }
    }
  }
  
  std::vector<buffer> _buffers;
};

Camera::Camera()
  : m_capture(new v4l("/dev/video0"))
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
  if(!m_capture || m_capture->is_open())
    return false;
  
  return m_capture->open();
}

bool Camera::isOpen() const
{
  return m_capture && m_capture->is_open();
}

bool Camera::close()
{
  if(!m_capture || !m_capture->is_open())
    return false;
  
  m_capture->close();
  
  return true;
}

bool Camera::update()
{
  if(!this->isOpen())
    return false;
  
  // Try to get a new frame
  m_capture->next(m_image);
    
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
  //m_capture->set(CV_CAP_PROP_FRAME_WIDTH, width);
}

void Camera::setHeight(const int height)
{
  //m_capture->set(CV_CAP_PROP_FRAME_HEIGHT, height);
}

void Camera::setConfig(const CameraConfig *config)
{
  // Cleanup previous channels
  auto it = m_channels.begin();
  for(; it != m_channels.end(); ++it) delete *it;
  m_channels.clear();
  
  // Delete previous config and set new config
  delete m_config;
  m_config = config;
  
  // Populate channels based on new config
  if(m_config)
  {
    const int numChannels = m_config->numChannels();
    for(int chNum = 0; chNum < numChannels; ++chNum)
      m_channels.push_back(new Channel(*m_config, chNum));
  }
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