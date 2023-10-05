#include "serialbase.h"

#include "stats.h"
#include "util.h"

#if LIN
#include <fcntl.h> 
#include <errno.h> 
#include <termios.h> 
#include <unistd.h>
#endif

//======================================================
//======================================================
SerialBase::SerialBase()
{
	this->connected = false;
  this->writeBufferCount = 0;
}


//======================================================
//======================================================
bool SerialBase::WriteData(unsigned char *buffer, unsigned int nbChar)
{
  if (!this->IsConnected()) return false;

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
bool SerialBase::IsConnected()
{
	return this->connected;
}

