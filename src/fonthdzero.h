#pragma once

#include "fontbase.h"


#include "config.h"
#ifdef __linux
#include <sys/types.h>
#include <fcntl.h>
#endif

//======================================================
//======================================================
class FontHDZero : public FontBase
{
protected:

public:
	FontHDZero(const char* fileName, const char* fontName);
	~FontHDZero();

  void drawChar(uint16_t code, float x1, float y1, float width, float height) override;

  bool isAnalog() override
  {
    return false;
  }

};
