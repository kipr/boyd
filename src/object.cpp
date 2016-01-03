#include "object.hpp"

Object::Object(const unsigned centroidX,
  const unsigned centroidY,
  const unsigned bBoxX,
  const unsigned bBoxY,
  const unsigned bBoxWidth,
  const unsigned bBoxHeight,
  const unsigned confidence)
  : centroidX(centroidX),
  centroidY(centroidY),
  bBoxX(bBoxX),
  bBoxY(bBoxY),
  bBoxWidth(bBoxWidth),
  bBoxHeight(bBoxHeight),
  bBoxArea(bBoxWidth * bBoxHeight),
  confidence(confidence)
  {
  }

Object::Object(const Object &rhs)
  : centroidX(rhs.centroidX),
  centroidY(rhs.centroidY),
  bBoxX(rhs.bBoxX),
  bBoxY(rhs.bBoxY),
  bBoxWidth(rhs.bBoxWidth),
  bBoxHeight(rhs.bBoxHeight),
  bBoxArea(rhs.bBoxArea),
  confidence(rhs.confidence)
{
}