#include "fontwalksnail.h"

#include "util.h"
#include "stb/stb_image.h"

#define OSD_CHAR_WIDTH_24 24
#define OSD_CHAR_HEIGHT_24 36

#define OSD_CHAR_WIDTH_36 36
#define OSD_CHAR_HEIGHT_36 54

#define CHARS_PER_FILE 512

FontWalksnail::FontWalksnail(std::filesystem::path path)
{
  strcpy(this->name, ("Walksnail: " + path.filename().replace_extension().string()).c_str());

  int width, height, channels;
  unsigned int charByteWidth, charByteSize;

  uint8_t* image = stbi_load(path.string().c_str(), &width, &height, &channels, 0);
  if (!image)
  {
    LOG("Unable to load font file: ", path, "Error: ", stbi_failure_reason());
    return;
  }

  if ((width != OSD_CHAR_WIDTH_24) && (width != OSD_CHAR_WIDTH_36))
  {
    LOG("Unexpected image size: %s\n", path);
    return;
  }

  this->charWidth = width;
  this->charHeight = height / CHARS_PER_FILE;
  charByteSize = this->charWidth * this->charHeight * BYTES_PER_PIXEL_RGBA;
  charByteWidth = this->charWidth * BYTES_PER_PIXEL_RGBA;

  if ((this->charWidth == OSD_CHAR_WIDTH_24) && (this->charHeight != OSD_CHAR_HEIGHT_24))
  {
    LOG("Unexpected image size: %s\n", path);
    stbi_image_free(image);
    return;
  }

  if ((this->charWidth == OSD_CHAR_WIDTH_36) && (this->charHeight != OSD_CHAR_HEIGHT_36))
  {
    LOG("Unexpected image size: %s\n", path);
    stbi_image_free(image);
    return;
  }

  for (int charIndex = 0; charIndex < CHARS_PER_FILE; charIndex++)
  {
    // Flip character bitmap to convert to OpenGl coords (0, 0) = left down
    unsigned int charBegin = charIndex * charByteSize;
    std::vector<uint8_t> texture(charByteSize);
    int targetIdx = charByteSize - charByteWidth;
    for (unsigned int j = 0; j < charByteSize; j += charByteWidth) {
      std::copy_n(image + charBegin + j, charByteWidth, texture.begin() + targetIdx);
      targetIdx -= charByteWidth;
    }
    this->textures.push_back(texture);
  }
  stbi_image_free(image);
}

int FontWalksnail::getCols()
{
  return 53;
}

int FontWalksnail::getRows()
{
  return 20;
}

bool FontWalksnail::isAnalog()
{
  return false;
}
