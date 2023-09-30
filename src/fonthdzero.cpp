#include "lodepng.h"

#include "fonthdzero.h"
#include "util.h"

#define OSD_CHAR_WIDTH_24 24
#define OSD_CHAR_HEIGHT_24 36

#define OSD_CHAR_WIDTH_36 36
#define OSD_CHAR_HEIGHT_36 54

#define CHARS_PER_TEXTURE_ROW 14

#pragma pack(1)
struct BMPFileHeader {
  uint16_t fileType;       // Signature: 'BM' for Windows BMP
  uint32_t fileSize;       // Total file size in bytes
  uint16_t reserved1;      // Reserved; set to 0
  uint16_t reserved2;      // Reserved; set to 0
  uint32_t dataOffset;     // Offset of pixel data from the beginning of the file

  uint32_t headerSize;     // Size of the info header (40 bytes for BMP v3)
  int32_t width;           // Width of the image in pixels
  int32_t height;          // Height of the image in pixels (positive for top-down)
  uint16_t colorPlanes;    // Number of color planes (must be 1)
  uint16_t bitsPerPixel;   // Number of bits per pixel (usually 24 for RGB)
  uint32_t compression;    // Compression method (usually 0 for no compression)
  uint32_t imageSize;      // Size of the image data in bytes
  int32_t xPixelsPerMeter; // Horizontal resolution (pixels per meter)
  int32_t yPixelsPerMeter; // Vertical resolution (pixels per meter)
  uint32_t colorsUsed;     // Number of colors in the color palette (usually 0 for full color)
  uint32_t importantColors; // Number of important colors (usually 0)
};
#pragma pack()



//======================================================
//======================================================
static bool decodeBMP( unsigned char** image, unsigned* width, unsigned* height, const char* fileName)
{
  std::ifstream file(fileName, std::fstream::in | std::fstream::binary);

  if (!file)
  {
    LOG("Unable  to open file: %s\n", fileName);
    return false;
  }

  BMPFileHeader header;
  file.read(reinterpret_cast<char*>(&header), sizeof(header));

  if (
    (header.fileType != 0x4D42) ||
    (header.headerSize != 40) ||
    (header.colorPlanes != 1) ||
    (header.bitsPerPixel != 24) ||
    (header.compression != 0) 
    )
  {
    LOG("Inknown file format: %s\n", fileName);
    return false;
  }

  // Get the size of the file
  file.seekg(0, std::ios::end);
  std::streampos fileSize = file.tellg();
  file.seekg(header.dataOffset, std::ios::beg);

  *image = new unsigned char[static_cast<int>(fileSize) - header.dataOffset];
  *width = header.width;
  *height = header.height;

  file.read(reinterpret_cast<char*>(*image), static_cast<int>(fileSize) - header.dataOffset);

  file.close();
  return true;
}


//======================================================
//======================================================
FontHDZero::FontHDZero(const char* fileName, const char* fontName) : FontBase(fontName)
{
  strcpy(this->name, fontName);

  unsigned char* image = 0;
  unsigned width, height;

  bool res = decodeBMP(&image, &width, &height, fileName);
  if (!res)
  {
    return;
  }

  if ((width != OSD_CHAR_WIDTH_24*16) && (width != OSD_CHAR_WIDTH_36*16))
  {
    LOG("Unexpected image size: %s\n", fileName);
    return;
  }

  this->charWidth = width / 16;
  this->charHeight = height / 32;

  if ((this->charWidth == OSD_CHAR_WIDTH_24*16) && (this->charHeight != OSD_CHAR_HEIGHT_24*16))
  {
    LOG("Unexpected image size: %s\n", fileName);
    return;
  }

  if ((this->charWidth == OSD_CHAR_WIDTH_36*16) && (this->charHeight != OSD_CHAR_HEIGHT_36*16))
  {
    LOG("Unexpected image size: %s\n", fileName);
    return;
  }

  this->calculateTextureHeight(this->charWidth * CHARS_PER_TEXTURE_ROW, this->charHeight * (512 + CHARS_PER_TEXTURE_ROW - 1) / CHARS_PER_TEXTURE_ROW);

  uint8_t* buffer = new uint8_t[this->fontTextureWidth * this->fontTextureHeight * 4];

  memset((void*)buffer, 0, this->fontTextureWidth * this->fontTextureHeight * 4);

  for (int charIndex = 0; charIndex < 512; charIndex++)
  {
    int ix = (charIndex % 16) * this->charWidth;
    int iy = ( charIndex / 16 ) * this->charHeight;

    int tx = (charIndex % CHARS_PER_TEXTURE_ROW) * this->charWidth;
    int ty = (charIndex / CHARS_PER_TEXTURE_ROW) * this->charHeight;

    for (unsigned int y = 0; y < this->charHeight; y++)
    {
      const uint8_t* pi = image + (height-1 - (iy+y)) * width * 3 + ix * 3;
      uint8_t* pt = buffer + (ty+y) * this->fontTextureWidth * 4 + tx * 4;

      for (unsigned int x = 0; x < this->charWidth; x++)
      {
        if ( (pi[0] != 0x7f) || (pi[1] != 0x7f) || (pi[2] != 0x7f) )
        {
          pt[0] = pi[2];
          pt[1] = pi[1];
          pt[2] = pi[0];
          pt[3] = 255;
        }

        pi += 3;
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

  delete[] image;
}


//======================================================
//======================================================
FontHDZero::~FontHDZero()
{
}

//======================================================
//======================================================
void FontHDZero::drawChar(uint16_t code, float x1, float y1, float width, float height)
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