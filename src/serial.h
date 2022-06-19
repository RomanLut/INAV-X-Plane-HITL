#pragma once 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SERIAL_BUFFER_SIZE 512

//======================================================
//======================================================
class Serial
{
private:
	HANDLE hSerial;
	bool connected;
	COMSTAT status;
	DWORD errors;

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
