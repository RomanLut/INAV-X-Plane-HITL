#pragma once

#include "config.h"
#ifdef __linux
#include <sys/types.h>
#include <fcntl.h>
#endif

#define SERIAL_BUFFER_SIZE 512

//======================================================
//======================================================
class Serial
{
private:
	bool connected;

#if IBM
  HANDLE hSerial;
#elif LIN
  int fd;
#endif

  uint8_t writeBuffer[SERIAL_BUFFER_SIZE];
  int writeBufferCount;

public:
	Serial(char *portName);
	~Serial();
	int ReadData(unsigned char *buffer, unsigned int nbChar);
	bool WriteData(unsigned char *buffer, unsigned int nbChar);
	bool IsConnected();

  void flushOut();
};
