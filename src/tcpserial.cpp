#include "serial.h"

#include "stats.h"
#include "util.h"
#include "tcpserial.h"

//======================================================
//======================================================
void TCPSerial::OpenConnection(char *address, int port)
{
#if IBM
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
  {
    LOG("Failed to initialize Winsock\n");
    return;
  }
#endif

#if IBM
  this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
#else
  this->sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif
  if (this->sockfd == INVALID_SOCKET)
  {
    LOG("Failed to create socket\n");
#if IBM
    WSACleanup(); // Cleanup on Windows
#endif
    return;
  }

  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port); 
  serverAddr.sin_addr.s_addr = inet_addr(address); 

  if (connect(this->sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
  {
    LOG("Failed to connect to the server\n");
    this->CloseConnection();
    return;
  }

#if LIN
  int flags = fcntl(sockfd, F_GETFL, 0);
  fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
#endif

#if IBM
  unsigned long ul = 1;
  if ( ioctlsocket(this->sockfd, FIONBIO, (unsigned long *)&ul) == SOCKET_ERROR)
  {
    LOG("Failed to set socket mode to non-blocking\n");
    this->CloseConnection();
  }
#endif

  this->connected = true;
}

//======================================================
//======================================================
void TCPSerial::CloseConnection()
{
  if (!this->connected) return;

#ifdef _WIN32
  closesocket(sockfd);
  WSACleanup();
#else
  close(sockfd);
#endif
  this->connected = false;
}

//======================================================
//======================================================
int TCPSerial::ReadData(unsigned char *buffer, unsigned int nbChar)
{
	if (nbChar == 0) return 0;
  if (!this->connected) return 0;

#ifdef _WIN32
  int bytesRead = recv(this->sockfd, (char*)buffer, nbChar, 0);
#else
  int bytesRead = recv(sockfd, buffer, nbChar, MSG_DONTWAIT);
#endif
  if (bytesRead > 0)
  {
    return bytesRead;
  }
  else if (bytesRead == 0)
  {
    this->CloseConnection();
    return 0;
  }
  else
  {
    //EWOULDBLOCK/EAGAIN
    return 0; 
  }
}

//======================================================
//======================================================
void TCPSerial::flushOut()
{
  if (!this->connected) return;

  if (this->writeBufferCount > 0)
  {
#if IBM
    int bytesSent = send(this->sockfd, (const char*)this->writeBuffer, this->writeBufferCount, 0);
#else
    int bytesSent = write(this->sockfd, (const char*)this->writeBuffer, this->writeBufferCount);
#endif

    if (bytesSent == SOCKET_ERROR)
    {
      LOG("Failed to send data\n");
      this->CloseConnection();
    }

    this->writeBufferCount = 0;
  }
}

//======================================================
//======================================================
TCPSerial::~TCPSerial()
{
  if (this->connected)
  {
    this->CloseConnection();
    this->connected = false;
  }
}
