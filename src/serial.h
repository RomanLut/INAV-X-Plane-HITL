#pragma once

#include "config.h"
#include "serialbase.h"

#ifdef __linux
#include <sys/types.h>
#include <fcntl.h>
#endif


//======================================================
//======================================================
class Serial : public SerialBase
{
private:
#if IBM
  HANDLE hSerial;
#elif LIN || APL
  int fd;
#endif
  void CloseConnection();

public:
  ~Serial() override;
  void OpenConnection(char *portName);
	int ReadData(unsigned char *buffer, unsigned int nbChar) override;
  void flushOut() override;
};
