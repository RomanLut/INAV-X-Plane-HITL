#include "lodepng.h"

#include "fontanalog.h"
#include "util.h"

#define FONT_IMAGE_WIDTH   209
#define FONT_IMAGE_HEIGHT  609

#define FONT_TEXTURE_WIDTH   512
#define FONT_TEXTURE_HEIGHT  1024

#define OSD_CHAR_WIDTH 12
#define OSD_CHAR_HEIGHT 18

//======================================================
//======================================================
FontAnalog::FontAnalog(const char* fileName, const char* fontName) : FontBase( fontName )
{
  this->charWidth = OSD_CHAR_WIDTH;
  this->charHeight = OSD_CHAR_HEIGHT;

  unsigned char* image = 0;
  unsigned width, height;

  unsigned int error = lodepng_decode32_file(&image, &width, &height, fileName);
  if (error)
  {
    LOG("error %u: %s\n", error, lodepng_error_text(error));
    return;
  }

  if (width != FONT_IMAGE_WIDTH || height != FONT_IMAGE_HEIGHT)
  {
    LOG("Unexpected image size: %s\n", fileName);
    return;
  }

  this->fontTextureWidth = FONT_TEXTURE_WIDTH;
  this->fontTextureHeight = FONT_TEXTURE_HEIGHT;

  uint8_t* buffer = new uint8_t[this->fontTextureWidth * this->fontTextureHeight * 4];

  memset((void*)buffer, 0, this->fontTextureWidth * this->fontTextureHeight * 4);

  for (unsigned int cy = 0; cy < height; cy++)
  {
    if ((cy % (OSD_CHAR_HEIGHT + 1)) == 0) continue;

    const uint8_t* pi = image + cy * width * 4;
    uint8_t* pt = buffer + cy * this->fontTextureWidth * 4;

    for (unsigned int cx = 0; cx < width; cx++)
    {
      if (((cx % (OSD_CHAR_WIDTH + 1)) != 0) && (pi[0] != 128))
      {
        pt[0] = pt[1] = pt[2] = pi[0];
        pt[3] = 255;
      }

      pi += 4;
      pt += 4;
    }
  }

  XPLMGenerateTextureNumbers(&this->fontTextureId, 1);
  XPLMBindTexture2d(this->fontTextureId, 0);
  glTexImage2D(
    GL_TEXTURE_2D,
    0,                   // mipmap level
    GL_RGBA,             // internal format for the GL to use.  (We could ask for a floating point tex or 16-bit tex if we were crazy!)
    this->fontTextureWidth,
    this->fontTextureHeight,
    0,                   // border size
    GL_RGBA,             // format of color we are giving to GL
    GL_UNSIGNED_BYTE,    // encoding of our data
    buffer);

  free(image);
}


//======================================================
//======================================================
FontAnalog::~FontAnalog()
{
}

//======================================================
//======================================================
void FontAnalog::drawChar(uint16_t code, float x1, float y1, float width, float height)
{
  if (this->fontTextureId == 0) return;

  int code9 = code % 0xff;

  int px = 1 + (code % 16) * (OSD_CHAR_WIDTH + 1);
  int py = 1 + (code / 16) * (OSD_CHAR_HEIGHT + 1);

  float u1 = (float)px;
  float u2 = u1 + OSD_CHAR_WIDTH;

  float v1 = (float)py;
  float v2 = v1 + OSD_CHAR_HEIGHT;

  u1 /= FONT_TEXTURE_WIDTH;
  v1 /= FONT_TEXTURE_HEIGHT;
  u2 /= FONT_TEXTURE_WIDTH;
  v2 /= FONT_TEXTURE_HEIGHT;

  float x2 = x1 + width;
  float y2 = y1 - height;

  glTexCoord2f(u1, v1);        glVertex2f(x1, y1);
  glTexCoord2f(u2, v1);        glVertex2f(x2, y1);
  glTexCoord2f(u2, v2);        glVertex2f(x2, y2);
  glTexCoord2f(u1, v2);        glVertex2f(x1, y2);
}