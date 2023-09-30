#pragma once

#include "fontbase.h"


#include "config.h"
#ifdef __linux
#include <sys/types.h>
#include <fcntl.h>
#endif

//======================================================
//======================================================
class FontWtfos : public FontBase
{
private:

  unsigned char* loadFontFile(const char* fileName);

protected:

public:
	FontWtfos(const char* dirName, const char* fontName);
	~FontWtfos();

  void drawChar(uint16_t code, float x1, float y1, float width, float height) override;

  bool isAnalog() override
  {
    return false;
  }

};
