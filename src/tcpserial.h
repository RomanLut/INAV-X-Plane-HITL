#pragma once

#include "config.h"
#include "serialbase.h"

//======================================================
//======================================================
class TCPSerial: public SerialBase
{
private:
  SOCKET sockfd = INVALID_SOCKET;

  void CloseConnection();

public:
  ~TCPSerial() override;
  void OpenConnection(char *address, int port);
	int ReadData(unsigned char *buffer, unsigned int nbChar) override;
  void flushOut() override;
};
