#pragma once

#include "config.h"

#define BYTES_PER_PIXEL_RGBA 4
#define MAX_FONT_NAME MAX_PATH

class FontBase {

protected:
  std::vector<std::vector<uint8_t>> textures = std::vector<std::vector<uint8_t>>();
  unsigned int charWidth = 0;
  unsigned int charHeight = 0;

public:

  char name[MAX_FONT_NAME];

  unsigned int getCharWidth();
  unsigned int getCharHeight();
  std::vector<std::vector<uint8_t>> getTextures();
  virtual int getCols() = 0;
  virtual int getRows() = 0;
  virtual bool isAnalog() = 0;
};