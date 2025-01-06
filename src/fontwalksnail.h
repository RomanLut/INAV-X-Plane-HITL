#pragma once

#include "fontbase.h"

class FontWalksnail : public FontBase {
public:
  FontWalksnail() = default;
  FontWalksnail(std::filesystem::path path);
  int getCols();
  int getRows();
  bool isAnalog();
};