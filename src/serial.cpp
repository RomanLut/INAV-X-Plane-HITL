#include "serial.h"

#include "stats.h"

//======================================================
//======================================================
Serial::Serial(char *portName)
{
	this->connected = false;

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
			printf("  ERROR: Handle was not attached. Reason: %s not available.\n", portName);

		}
		else
		{
			printf("  Couldn't connect to COM port, unknown why..\n");
		}
	}
	else
	{
		DCB dcbSerialParams = { 0 };
		if (!GetCommState(this->hSerial, &dcbSerialParams))
		{
			printf("  failed to get current serial parameters!\n");
		}
		else
		{
			dcbSerialParams.BaudRate = 115200;
			dcbSerialParams.ByteSize = 8;
			dcbSerialParams.StopBits = ONESTOPBIT;
			dcbSerialParams.Parity = NOPARITY;

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

  this->writeBufferCount = 0;
}

//======================================================
//======================================================
Serial::~Serial()
{
	if (this->connected)
	{
		this->connected = false;
		CloseHandle(this->hSerial);
	}
}

//======================================================
//======================================================
int Serial::ReadData(unsigned char *buffer, unsigned int nbChar)
{
	if (nbChar == 0) return 0;
	DWORD bytesRead;
	unsigned int toRead;

	ClearCommError(this->hSerial, &this->errors, &this->status);

	if (this->status.cbInQue>0)
	{
    toRead = (this->status.cbInQue>nbChar) ? nbChar : this->status.cbInQue;

		if (ReadFile(this->hSerial, buffer, toRead, &bytesRead, NULL) && bytesRead != 0)
		{
      g_stats.serialBytesReceived += bytesRead;
      g_stats.serialPacketsReceived += 1;
      return bytesRead;
		}
	}

	return 0;
}

//======================================================
//======================================================
bool Serial::WriteData(unsigned char *buffer, unsigned int nbChar)
{
  if (this->writeBufferCount + nbChar < SERIAL_BUFFER_SIZE)
  {
    for (unsigned int i = 0; i < nbChar; i++)
    {
      this->writeBuffer[this->writeBufferCount++] = *(buffer++);
    }

    return true;
  }
  else
  {
    return false;
  }
}

//======================================================
//======================================================
bool Serial::IsConnected()
{
	return this->connected;
}

//======================================================
//======================================================
void Serial::flushOut()
{
  if (this->writeBufferCount > 0)
  {
    DWORD bytesSend;

    uint32_t t = GetTickCount();

    if (!WriteFile(this->hSerial, (void *)this->writeBuffer, this->writeBufferCount, &bytesSend, 0))
    {
      ClearCommError(this->hSerial, &this->errors, &this->status);
      return;
    }

    g_stats.serialBytesSent += this->writeBufferCount;
    g_stats.serialPacketsSent += 1;

    this->writeBufferCount = 0;
  }
}