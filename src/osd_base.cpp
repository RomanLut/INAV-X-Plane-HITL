#include "osd_base.h"
#include "map.h"

#define SYM_LAT                     0x03  // 003 GPS LAT
#define SYM_LON                     0x04  // 004 GPS LON
#define SYM_ZERO_HALF_TRAILING_DOT  0xA1  // 161 to 170 Numbers with trailing dot
#define SYM_ZERO_HALF_LEADING_DOT   0xB1  // 177 to 186 Numbers with leading dot

float OSDBase::extractFloat(int x, int y)
{
  int cols = this->getColCount();
  int rowEndIndex = (x / cols) * cols + cols;

  int res = 0;
  bool neg = false;
  int div = 1;
  bool p = false;

  x++;
  while (x < rowEndIndex)
  {
    int ch = this->getChar(y, x);

    if (ch == '-')
    {
      neg = true;
    }
    else if ((ch >= '0') && (ch <= '9'))
    {
      res = res * 10 + (ch - '0');
    }
    else if ((ch >= SYM_ZERO_HALF_TRAILING_DOT) && (ch <= (SYM_ZERO_HALF_TRAILING_DOT + 10)))
    {
      res = res * 10 + (ch - SYM_ZERO_HALF_TRAILING_DOT);
    }
    else if ((ch >= SYM_ZERO_HALF_LEADING_DOT) && (ch <= (SYM_ZERO_HALF_LEADING_DOT + 10)))
    {
      res = res * 10 + (ch - SYM_ZERO_HALF_LEADING_DOT);
      p = true;
    }
    else
    {
      break;
    }

    x++;
    if (p) div *= 10;
  }

  float resf = res * 1.0f / div;

  return neg ? -resf : resf;
}

void OSDBase::extractLatLon()
{
  float lat = 0;
  float lon = 0;

  int flags = 0;

  for (int y = 0; y < this->getRowCount(); y++)
  {
    for (int x = 0; x < this->getColCount(); x++)
    {

      if (this->getChar(y, x) == SYM_LAT)
      {
        lat = this->extractFloat(x, y);
        flags |= 1;
      }
      if (this->getChar(y, x) == SYM_LON)
      {
        lon = this->extractFloat(x, y);
        flags |= 2;
      }
    }
  }

  if (flags == 3)
  {
    g_map.addLatLonOSD(lat, lon);
  }
}