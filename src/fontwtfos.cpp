#include "lodepng.h"

#include "fontwtfos.h"
#include "util.h"

#define CHAR_WIDTH			  36
#define CHAR_HEIGHT			  54
#define CHARS_PER_FILE		256
#define BYTES_PER_PIXEL		4

#define CHAR_SIZE (CHAR_HEIGHT * CHAR_WIDTH * BYTES_PER_PIXEL)
#define FONT_FILE_SIZE (CHAR_SIZE * CHARS_PER_FILE)

#define CHARS_PER_TEXTURE_ROW 14

//======================================================
//======================================================
FontWtfos::FontWtfos(const char* dirName, const char* fontName) : FontBase(fontName)
{
  strcpy(this->name, fontName);

  char s[MAX_PATH];
  char filePartName[MAX_PATH];

  strcpy(s, dirName);
  strcat(s, "\\font_inav.bin");
  buildAssetFilename(filePartName, s);
  unsigned char* part1 = this->loadFontFile(filePartName);
  if (!part1) return;

  strcpy(s, dirName);
  strcat(s, "\\font_inav_2.bin");
  buildAssetFilename(filePartName, s);
  unsigned char* part2 = this->loadFontFile(filePartName);
  if (!part2)
  {
    free(part1);
    return;
  }

  this->charWidth = CHAR_WIDTH;
  this->charHeight = CHAR_HEIGHT;

  this->calculateTextureHeight(this->charWidth * CHARS_PER_TEXTURE_ROW, this->charHeight * (512 + CHARS_PER_TEXTURE_ROW - 1) / CHARS_PER_TEXTURE_ROW);

  uint8_t* buffer = new uint8_t[this->fontTextureWidth * this->fontTextureHeight * 4];
  memset((void*)buffer, 0, this->fontTextureWidth * this->fontTextureHeight * 4);

  for (int charIndex = 0; charIndex < 512; charIndex++)
  {
    int ix = 0;
    int iy = charIndex * this->charHeight;
    if (charIndex >= 256)
    {
      iy = (charIndex-256) * this->charHeight;
    }

    int tx = (charIndex % CHARS_PER_TEXTURE_ROW) * this->charWidth;
    int ty = (charIndex / CHARS_PER_TEXTURE_ROW) * this->charHeight;

    for (unsigned int y = 0; y < this->charHeight; y++)
    {
      uint8_t* pi = part1 + (iy+y) * CHAR_WIDTH * 4 + ix * 4;
      if (charIndex >= 256)
      {
        pi = part2 + (iy + y) * CHAR_WIDTH * 4 + ix * 4;
      }

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

  /*
  strcpy(s, dirName);
  strcat(s, "\\texture.png");
  buildAssetFilename(filePartName, s);
  lodepng_encode_file(filePartName, buffer, this->fontTextureWidth, this->fontTextureHeight, LCT_RGBA, 8);
  */

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

  free(part1);
  free(part2);
}


//==============================================================
//==============================================================
unsigned char* FontWtfos::loadFontFile(const char* fileName)
{
  unsigned char* buffer = (unsigned char*)malloc(FONT_FILE_SIZE);

  std::fstream file(fileName, std::fstream::in | std::fstream::binary | std::fstream::ate);
  int size = 0;

  if (!file.is_open())
  {
    LOG("Unable to open file: %s\n", fileName);
    return NULL;
  }

  size = static_cast<int>(file.tellg());
  if (size != FONT_FILE_SIZE)
  {
    LOG("Incorrect file size: %s\n", fileName);
    file.close();
    return NULL;
  }

  file.seekg(0);
  file.read((char*)buffer, size);

  file.close();

  return buffer;
}

//======================================================
//======================================================
FontWtfos::~FontWtfos()
{
}

//======================================================
//======================================================
void FontWtfos::drawChar(uint16_t code, float x1, float y1, float width, float height)
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