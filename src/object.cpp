#include "object.hpp"

Object::Object(const unsigned centroidX,
  const unsigned centroidY,
  const unsigned bBoxX,
  const unsigned bBoxY,
  const unsigned bBoxWidth,
  const unsigned bBoxHeight,
  const double confidence,
  const char *data,
  const size_t &dataLength)
  : centroidX(centroidX),
  centroidY(centroidY),
  bBoxX(bBoxX),
  bBoxY(bBoxY),
  bBoxWidth(bBoxWidth),
  bBoxHeight(bBoxHeight),
  bBoxArea(bBoxWidth * bBoxHeight),
  confidence(confidence),
  data(0),
  dataLength(dataLength)
  {
    if(!data) return;
    
    this->data = new char[dataLength];
    memcpy(this->data, data, dataLength);
  }

Object::Object(const Object &rhs)
  : centroidX(rhs.centroidX),
  centroidY(rhs.centroidY),
  bBoxX(rhs.bBoxX),
  bBoxY(rhs.bBoxY),
  bBoxWidth(rhs.bBoxWidth),
  bBoxHeight(rhs.bBoxHeight),
  bBoxArea(rhs.bBoxArea),
  confidence(rhs.confidence),
  data(0),
  dataLength(rhs.dataLength)
{
  if(!rhs.data) return;
  
  this->data = new char[dataLength + 1];
  memcpy(this->data, rhs.data, dataLength);
  this->data[dataLength] = 0;
}

Object::~Object()
{
  delete[] data;
}