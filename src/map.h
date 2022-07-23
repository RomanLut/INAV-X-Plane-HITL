#pragma once

#include "config.h"

#define MAX_MAP_POINTS  10000

//=======================================================
//=======================================================
typedef enum
{
  MMT_NONE        = 0,
  MMT_LAT_LON_OSD = 1,
  MMT_DEBUG_0_1   = 2
} TMapMarkType;


//=======================================================
//=======================================================
class TMap
{
public:

  void init();
  void destroy();

  void addDebug(int32_t lat, int32_t lon);
  void addLatLonOSD(float lat, float lon);

  void setMarkingType(TMapMarkType type);
  TMapMarkType getMarkingType();

private:

  typedef struct
  {
    float lat;
    float lon;
  } TCoords;

  XPLMMapLayerID layer = NULL;

  TMapMarkType markingType = MMT_LAT_LON_OSD;

  TCoords coords[MAX_MAP_POINTS];
  int pHead = 0;
  int pTail = 0;

  float crossLat = 0;
  float crossLon = 0;

  void createOurMapLayer(const char * mapIdentifier, void * refcon);
  static void createOurMapLayerStatic(const char * mapIdentifier, void * refcon);
  static void willBeDeletedStatic(XPLMMapLayerID layer, void * inRefcon);
  void drawMarkings(XPLMMapLayerID layer, const float * inMapBoundsLeftTopRightBottom, float zoomRatio, float mapUnitsPerUserInterfaceUnit, XPLMMapStyle mapStyle, XPLMMapProjectionID projection, void * inRefcon);
  static void drawMarkingsStatic(XPLMMapLayerID layer, const float * inMapBoundsLeftTopRightBottom, float zoomRatio, float mapUnitsPerUserInterfaceUnit, XPLMMapStyle mapStyle, XPLMMapProjectionID projection, void * inRefcon);

  void addPoint(float lat, float lon);
  void addPointEx(float lat, float lon);

};

extern TMap g_map;
