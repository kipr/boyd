#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_

#include <vector>

class Object
{
public:
  Object(const unsigned centroidX,
          const unsigned centroidY,
          const unsigned bBoxX,
          const unsigned bBoxY,
          const unsigned bBoxWidth,
          const unsigned bBoxHeight,
          const unsigned confidence);
  Object(const Object &rhs);
    
  unsigned centroidX;
  unsigned centroidY;
  unsigned bBoxX;
  unsigned bBoxY;
  unsigned bBoxWidth;
  unsigned bBoxHeight;
  unsigned long bBoxArea;
  unsigned confidence;
};

typedef std::vector<Object> ObjectVector;

#endif