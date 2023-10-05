#pragma once

#include "config.h"
#ifdef __linux
#include <sys/types.h>
#include <fcntl.h>
#endif

#define SERIAL_BUFFER_SIZE 512

//======================================================
//======================================================
class SerialBase
{
protected:
	bool connected;

  uint8_t writeBuffer[SERIAL_BUFFER_SIZE];
  int writeBufferCount;

public:
	SerialBase();

  virtual ~SerialBase() {};
  virtual int ReadData(unsigned char *buffer, unsigned int nbChar) = 0;
	bool WriteData(unsigned char *buffer, unsigned int nbChar);
	bool IsConnected();

  virtual void flushOut() = 0;
};
