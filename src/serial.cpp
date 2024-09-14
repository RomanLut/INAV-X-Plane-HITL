#include "serial.h"

#include "stats.h"
#include "util.h"

#if LIN
#include <fcntl.h> 
#include <errno.h> 
#include <termios.h> 
#include <unistd.h>
#endif

#define BAUDRATE  115200

//======================================================
//======================================================
void Serial::OpenConnection(char *portName)
{
#if IBM
  this->hSerial = CreateFile(portName,
    GENERIC_READ | GENERIC_WRITE,
    0,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL);

  if (this->hSerial == INVALID_HANDLE_VALUE)
  {
    if (GetLastError() == ERROR_FILE_NOT_FOUND)
    {
      LOG("ERROR: Handle was not attached. Reason: %s not available.", portName);
    }
    else
    {
      LOG("Couldn't connect to COM port, unknown error.");
    }
  }
  else
  {
    DCB dcbSerialParams = { 0 };
    if (!GetCommState(this->hSerial, &dcbSerialParams))
    {
      LOG("failed to get current serial parameters!");
    }
    else
    {
      dcbSerialParams.BaudRate = BAUDRATE;
      dcbSerialParams.ByteSize = 8;
      dcbSerialParams.StopBits = ONESTOPBIT;
      dcbSerialParams.Parity = NOPARITY;

      // Disable software flow control (XON/XOFF)
      dcbSerialParams.fOutX = FALSE;
      dcbSerialParams.fInX = FALSE;

      // Disable hardware flow control (RTS/CTS)
      dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;
      dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE;

      // Disable any special processing of bytes
      dcbSerialParams.fBinary = TRUE;

      if (!SetCommState(hSerial, &dcbSerialParams))
      {
        printf("  ALERT: Could not set Serial Port parameters\n");
      }
      else
      {
        COMMTIMEOUTS comTimeOut;
        comTimeOut.ReadIntervalTimeout = MAXDWORD;
        comTimeOut.ReadTotalTimeoutMultiplier = 0;
        comTimeOut.ReadTotalTimeoutConstant = 0;
        comTimeOut.WriteTotalTimeoutMultiplier = 0;
        comTimeOut.WriteTotalTimeoutConstant = 300;
        SetCommTimeouts(hSerial, &comTimeOut);

        this->connected = true;
      }
    }
  }
#elif LIN || APL
  this->fd = open(portName, O_RDWR);
  if (fd == -1)
  {
    LOG("Couldn't connect to COM port %s", portName);
    return;
  }

  struct termios terminalOptions;
  memset(&terminalOptions, 0, sizeof(struct termios));
  tcgetattr(fd, &terminalOptions);

  cfmakeraw(&terminalOptions);

  cfsetispeed(&terminalOptions, BAUDRATE);
  cfsetospeed(&terminalOptions, BAUDRATE);

  terminalOptions.c_cflag = CREAD | CLOCAL;
  terminalOptions.c_cflag |= CS8;
  terminalOptions.c_cflag &= ~HUPCL;

  terminalOptions.c_lflag &= ~ICANON;
  terminalOptions.c_lflag &= ~ECHO; // Disable echo
  terminalOptions.c_lflag &= ~ECHOE; // Disable erasure
  terminalOptions.c_lflag &= ~ECHONL; // Disable new-line echo
  terminalOptions.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP

  terminalOptions.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
  terminalOptions.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

  terminalOptions.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
  terminalOptions.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

  terminalOptions.c_cc[VMIN] = 0;
  terminalOptions.c_cc[VTIME] = 0;

  int ret = tcsetattr(fd, TCSANOW, &terminalOptions);
  if (ret == -1)
  {
    LOG("Failed to configure device: %s", portName);
    return;
  }

  this->connected = true;
#endif
}

//======================================================
//======================================================
void Serial::CloseConnection()
{
#if IBM
	CloseHandle(this->hSerial);
#elif LIN || APL
  close(this->fd);
#endif
}

//======================================================
//======================================================
int Serial::ReadData(unsigned char *buffer, unsigned int nbChar)
{
	if (nbChar == 0) return 0;

#if IBM
  COMSTAT status;
  DWORD errors;
  DWORD bytesRead;
  unsigned int toRead;

	ClearCommError(this->hSerial, &errors, &status);
	if (status.cbInQue>0)
	{
    toRead = (status.cbInQue>nbChar) ? nbChar : status.cbInQue;

		if (ReadFile(this->hSerial, buffer, toRead, &bytesRead, NULL) && bytesRead != 0)
		{
      g_stats.serialBytesReceived += bytesRead;
      g_stats.serialPacketsReceived ++;
      return bytesRead;
		}
	}
  return 0;
#elif LIN || APL

  int bytesRead = read(this->fd, buffer, nbChar);
  g_stats.serialBytesReceived += bytesRead;
  g_stats.serialPacketsReceived++;
  return bytesRead;

#endif
}

//======================================================
//======================================================
void Serial::flushOut()
{
  if (this->writeBufferCount > 0)
  {
#if IBM
    COMSTAT status;
    DWORD errors;
    DWORD bytesSend;
    if (!WriteFile(this->hSerial, (void *)this->writeBuffer, this->writeBufferCount, &bytesSend, 0))
    {
      ClearCommError(this->hSerial, &errors, &status);
      return;
    }

    g_stats.serialBytesSent += this->writeBufferCount;
    g_stats.serialPacketsSent ++;

    this->writeBufferCount = 0;
#elif LIN || APL
    ssize_t written = write(this->fd, this->writeBuffer, this->writeBufferCount);
    if (written != this->writeBufferCount) {
      LOG("WARN: %i bytes written, but %i bytes requested");
    }

    g_stats.serialBytesSent += this->writeBufferCount;
    g_stats.serialPacketsSent++;

    this->writeBufferCount = 0;
#endif
  }
}

//======================================================
//======================================================
Serial::~Serial()
{
  if (this->connected)
  {
    this->CloseConnection();
    this->connected = false;
  }
}
