#pragma once

#include "config.h"
#include "serialbase.h"

#if LIN
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h> 
#include <termios.h> 
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

#if IBM
#include <Winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif

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
