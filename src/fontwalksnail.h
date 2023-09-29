#pragma once

#include "fontbase.h"


#include "config.h"
#ifdef __linux
#include <sys/types.h>
#include <fcntl.h>
#endif

//======================================================
//======================================================
class FontWalksnail : public FontBase
{
protected:

public:
	FontWalksnail(const char* fileName, const char* fontName);
	~FontWalksnail();

  void drawChar(uint16_t code, float x1, float y1, float width, float height) override;

  bool isAnalog() override
  {
    return false;
  }

};
