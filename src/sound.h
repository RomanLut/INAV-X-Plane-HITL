#pragma once

#include "config.h"

//======================================================
//======================================================
class TSound
{
public:
  void init();
  void destroy();
  void play(const char* pFileName);
};

extern TSound g_sound;

