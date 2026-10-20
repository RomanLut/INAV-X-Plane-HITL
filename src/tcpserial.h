#pragma once

#include "config.h"
#include "serialbase.h"

//======================================================
//======================================================
class TCPSerial: public SerialBase
{
private:
  SOCKET sockfd = INVALID_SOCKET;
  bool connecting = false;
  uint32_t connectStartTime = 0;

  void CloseConnection();

public:
  typedef enum
  {
    CS_PENDING,     // non-blocking connect() still in progress
    CS_CONNECTED,
    CS_FAILED       // connect failed or timed out, or the connection was closed since
  } TConnectState;

  ~TCPSerial() override;
  void OpenConnection(char *address, int port);
  TConnectState PollConnect();
	int ReadData(unsigned char *buffer, unsigned int nbChar) override;
  void flushOut() override;
};
