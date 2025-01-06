#include "fonthdzero.h"

#include "util.h"
#include "stb/stb_image.h"

#define OSD_CHAR_WIDTH_24 24
#define OSD_CHAR_HEIGHT_24 36

#define OSD_CHAR_WIDTH_36 36
#define OSD_CHAR_HEIGHT_36 54

#define CHARS_PER_FONT_ROW 16
#define CHARS_PER_FONT_COLUMN 32
#define CHARS_PER_FILE (CHARS_PER_FONT_ROW * CHARS_PER_FONT_COLUMN)

FontHDZero::FontHDZero(std::filesystem::path path)
{
  strcpy(this->name, ("HDZero: " + path.filename().replace_extension().string()).c_str());
  unsigned int charByteSize = 0, charByteWidth = 0;
  int width, height, channels;
  uint8_t* image = stbi_load(path.string().c_str(), &width, &height, &channels, 0);
  if (!image)
  {
    LOG("Unable to load font file: ", path);
    return;
  }

  if ((width != OSD_CHAR_WIDTH_24 * CHARS_PER_FONT_ROW) && (width != OSD_CHAR_WIDTH_36 * CHARS_PER_FONT_ROW)) {
    LOG("Unexpected font size: %s\n", path);
    stbi_image_free(image);
    return;
  }

  this->charWidth = width / CHARS_PER_FONT_ROW;
  this->charHeight = height / CHARS_PER_FONT_COLUMN;
  charByteSize = this->charWidth * this->charHeight * BYTES_PER_PIXEL_RGBA;
  charByteWidth = this->charWidth * BYTES_PER_PIXEL_RGBA;

  if ((this->charWidth == OSD_CHAR_WIDTH_24) && (this->charHeight != OSD_CHAR_HEIGHT_24)) {
    LOG("Unexpected image size: %s\n", path);
    stbi_image_free(image);
    return;
  }

  if ((this->charWidth == OSD_CHAR_WIDTH_36) && (this->charHeight != OSD_CHAR_HEIGHT_36)) {
    LOG("Unexpected image size: %s\n", path);
    stbi_image_free(image);
    return;
  }

  for (int charIndex = 0; charIndex < CHARS_PER_FILE; charIndex++) {
    std::vector<uint8_t> character(charByteSize);
    int charHeigthIdx = charByteSize - charByteWidth;
    int ix = (charIndex % CHARS_PER_FONT_ROW) * this->charWidth;
    int iy = (charIndex / CHARS_PER_FONT_ROW) * this->charHeight;
    for (unsigned int y = 0; y < this->charHeight; y++) {
      for (unsigned int x = 0; x < this->charWidth; x++) {

        unsigned int idx = ((iy + y) * channels * width) + ((ix + x) * channels);
        uint8_t r = image[idx];
        uint8_t g = image[idx + 1];
        uint8_t b = image[idx + 2];

        if (r != 0x7f || g != 0x7f || b != 0x7f) {
          int cp = charHeigthIdx + (x * BYTES_PER_PIXEL_RGBA);
          character[cp] = r;
          character[cp + 1] = g;
          character[cp + 2] = b;
          character[cp + 3] = 0xff;
        }
      }
      charHeigthIdx -= charByteWidth;
    }
    this->textures.push_back(character);
  }
  stbi_image_free(image);
}

int FontHDZero::getCols()
{
  return 50;
}

int FontHDZero::getRows()
{
  return 18;
}

bool FontHDZero::isAnalog() {
  return false;
}
