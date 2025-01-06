#include "fontbase.h"

unsigned int FontBase::getCharWidth()
{
  return this->charWidth;
}

unsigned int FontBase::getCharHeight()
{
  return this->charHeight;
}

std::vector<std::vector<uint8_t>> FontBase::getTextures()
{
  return this->textures;
}
