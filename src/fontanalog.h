#pragma once

#include "fontbase.h"

//======================================================
//======================================================
class FontAnalog : public FontBase
{
public:
	FontAnalog() = default;
  FontAnalog(std::filesystem::path path);
  int getCols();
  int getRows();
  bool isAnalog();

};
