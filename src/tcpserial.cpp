#include "serial.h"

#include "stats.h"
#include "util.h"
#include "tcpserial.h"

#if LIN || APL
#include <sys/select.h>
#endif

#define TCP_CONNECT_TIMEOUT_MS  3000

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

#if APL
  //a send to a socket the peer has closed must fail with EPIPE, not kill X-Plane with SIGPIPE (Linux: MSG_NOSIGNAL in flushOut)
  int noSigPipe = 1;
  setsockopt(this->sockfd, SOL_SOCKET, SO_NOSIGPIPE, &noSigPipe, sizeof(noSigPipe));
#endif

  //non-blocking before connect(): a slow or unreachable host must not stall X-Plane's flight loop
#if IBM
  unsigned long ul = 1;
  if (ioctlsocket(this->sockfd, FIONBIO, &ul) == SOCKET_ERROR)
#else
  int flags = fcntl(this->sockfd, F_GETFL, 0);
  if (flags < 0 || fcntl(this->sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
#endif
  {
    LOG("Failed to set socket mode to non-blocking\n");
    this->CloseConnection();
    return;
  }

  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port); 
  serverAddr.sin_addr.s_addr = inet_addr(address); 

  if (connect(this->sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == 0)
  {
    this->connected = true;
    return;
  }

#if IBM
  bool pending = WSAGetLastError() == WSAEWOULDBLOCK;
#else
  bool pending = errno == EINPROGRESS;
#endif
  if (!pending)
  {
    LOG("Failed to connect to the server\n");
    this->CloseConnection();
    return;
  }

  //connect() completes in the background: PollConnect() polls it
  this->connecting = true;
  this->connectStartTime = GetTickCount();
}

//======================================================
//======================================================
TCPSerial::TConnectState TCPSerial::PollConnect()
{
  if (!this->connecting) return this->connected ? CS_CONNECTED : CS_FAILED;

  fd_set writeSet;
  fd_set errorSet;
  FD_ZERO(&writeSet);
  FD_ZERO(&errorSet);
  FD_SET(this->sockfd, &writeSet);
  FD_SET(this->sockfd, &errorSet);
  struct timeval timeout = { 0, 0 };
  int ready = select((int)this->sockfd + 1, NULL, &writeSet, &errorSet, &timeout);

  if (ready == 0)
  {
    if (GetTickCount() - this->connectStartTime < TCP_CONNECT_TIMEOUT_MS) return CS_PENDING;
    LOG("Connection timeout\n");
    this->CloseConnection();
    return CS_FAILED;
  }

  //writable means the handshake finished; SO_ERROR tells success from failure (Windows also flags failure in errorSet)
  int error = 0;
#if IBM
  int errorLength = sizeof(error);
  getsockopt(this->sockfd, SOL_SOCKET, SO_ERROR, (char*)&error, &errorLength);
#else
  socklen_t errorLength = sizeof(error);
  getsockopt(this->sockfd, SOL_SOCKET, SO_ERROR, &error, &errorLength);
#endif

  if (ready < 0 || error != 0 || FD_ISSET(this->sockfd, &errorSet))
  {
    LOG("Failed to connect to the server\n");
    this->CloseConnection();
    return CS_FAILED;
  }

  this->connecting = false;
  this->connected = true;
  return CS_CONNECTED;
}

//======================================================
//======================================================
void TCPSerial::CloseConnection()
{
  if (this->sockfd == INVALID_SOCKET) return;  //also closes a socket whose connect() failed

#ifdef _WIN32
  closesocket(sockfd);
  WSACleanup();
#else
  close(sockfd);
#endif
  this->sockfd = INVALID_SOCKET;
  this->connecting = false;
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
#elif LIN
    int bytesSent = send(this->sockfd, (const char*)this->writeBuffer, this->writeBufferCount, MSG_NOSIGNAL);  //EPIPE instead of a SIGPIPE that kills X-Plane
#else
    int bytesSent = send(this->sockfd, (const char*)this->writeBuffer, this->writeBufferCount, 0);  //SO_NOSIGPIPE is set in OpenConnection()
#endif

    if (bytesSent == SOCKET_ERROR)
    {
#if IBM
      bool full = WSAGetLastError() == WSAEWOULDBLOCK;
#else
      bool full = errno == EWOULDBLOCK || errno == EAGAIN;
#endif
      if (full) return;  //socket send buffer is full: keep the bytes and retry on the next flush

      LOG("Failed to send data\n");
      this->CloseConnection();
      this->writeBufferCount = 0;
      return;
    }

    //a partial send keeps the rest for the next flush, so the byte stream stays intact
    this->writeBufferCount -= bytesSent;
    memmove(this->writeBuffer, this->writeBuffer + bytesSent, this->writeBufferCount);
  }
}

//======================================================
//======================================================
TCPSerial::~TCPSerial()
{
  this->CloseConnection();  //also a socket still connecting
}
