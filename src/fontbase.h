#pragma once

#include "config.h"
#ifdef __linux
#include <sys/types.h>
#include <fcntl.h>
#endif

#define MAX_FONT_NAME 64

//======================================================
//======================================================
class FontBase
{
protected:
  int fontTextureId = 0;

  unsigned int charWidth;
  unsigned int charHeight;

  unsigned int fontTextureWidth;
  unsigned int fontTextureHeight;

  void calculateTextureHeight(unsigned int imageWidth, unsigned int imageHeight);

public:

  char name[MAX_FONT_NAME];

	FontBase(unsigned  int charWidth, unsigned  int charHeight);
	~FontBase();

  virtual void drawChar(uint16_t code, float x1, float y1, float width, float height) = 0;
  void destroy();
  void bindTexture();
  virtual bool isAnalog() = 0;
};
