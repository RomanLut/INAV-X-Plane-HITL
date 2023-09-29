#include "lodepng.h"

#include "fontwalksnail.h"
#include "util.h"

#define OSD_CHAR_WIDTH_24 24
#define OSD_CHAR_HEIGHT_24 36

#define OSD_CHAR_WIDTH_36 36
#define OSD_CHAR_HEIGHT_36 54

#define CHARS_PER_TEXTURE_ROW 14

//======================================================
//======================================================
FontWalksnail::FontWalksnail(const char* fileName, const char* fontName) : FontBase(fontName)
{
  strcpy(this->name, fontName);

  unsigned char* image = 0;
  unsigned width, height;

  unsigned int error = lodepng_decode32_file(&image, &width, &height, fileName);
  if (error)
  {
    LOG("error %u: %s\n", error, lodepng_error_text(error));
    return;
  }

  if ((width != OSD_CHAR_WIDTH_24) && (width != OSD_CHAR_WIDTH_36))
  {
    LOG("Unexpected image size: %s\n", fileName);
  }

  this->charWidth = width;
  this->charHeight = height / 512;

  if ((this->charWidth == OSD_CHAR_WIDTH_24) && (this->charHeight != OSD_CHAR_HEIGHT_24))
  {
    LOG("Unexpected image size: %s\n", fileName);
  }

  if ((this->charWidth == OSD_CHAR_WIDTH_36) && (this->charHeight != OSD_CHAR_HEIGHT_36))
  {
    LOG("Unexpected image size: %s\n", fileName);
  }

  this->calculateTextureHeight(this->charWidth * CHARS_PER_TEXTURE_ROW, this->charHeight * (512 + CHARS_PER_TEXTURE_ROW - 1) / CHARS_PER_TEXTURE_ROW);

  uint8_t* buffer = new uint8_t[this->fontTextureWidth * this->fontTextureHeight * 4];

  memset((void*)buffer, 0, this->fontTextureWidth * this->fontTextureHeight * 4);

  for (int charIndex = 0; charIndex < 512; charIndex++)
  {
    int ix = 0;
    int iy = charIndex * this->charHeight;

    int tx = (charIndex % CHARS_PER_TEXTURE_ROW) * this->charWidth;
    int ty = (charIndex / CHARS_PER_TEXTURE_ROW) * this->charHeight;

    for (unsigned int y = 0; y < this->charHeight; y++)
    {
      const uint8_t* pi = image + (iy+y) * width * 4 + ix * 4;
      uint8_t* pt = buffer + (ty+y) * this->fontTextureWidth * 4 + tx * 4;

      for (unsigned int x = 0; x < this->charWidth; x++)
      {
        pt[0] = pi[0];
        pt[1] = pi[1];
        pt[2] = pi[2];
        pt[3] = pi[3];

        pi += 4;
        pt += 4;
      }
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
FontWalksnail::~FontWalksnail()
{
}

//======================================================
//======================================================
void FontWalksnail::drawChar(uint16_t code, float x1, float y1, float width, float height)
{
  if (this->fontTextureId == 0) return;

  int code9 = code % 0xff;

  int px = (code % CHARS_PER_TEXTURE_ROW) * this->charWidth;
  int py = (code / CHARS_PER_TEXTURE_ROW) * this->charHeight;

  float u1 = (float)px;
  float u2 = u1 + this->charWidth;

  float v1 = (float)py;
  float v2 = v1 + this->charHeight;

  u1 /= this->fontTextureWidth;
  v1 /= this->fontTextureHeight;
  u2 /= this->fontTextureWidth;
  v2 /= this->fontTextureHeight;

  float x2 = x1 + width;
  float y2 = y1 - height;

  glTexCoord2f(u1, v1);        glVertex2f(x1, y1);
  glTexCoord2f(u2, v1);        glVertex2f(x2, y1);
  glTexCoord2f(u2, v2);        glVertex2f(x2, y2);
  glTexCoord2f(u1, v2);        glVertex2f(x1, y2);
}