#pragma once

#include "config.h"
#include "msp.h"

#define MAX_MAP_POINTS  10000
#define MAX_WAYPOINTS   255

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

  void clearTracks();
  void startDownloadWaypoints();
  void onWPInfo(const TMSPWPInfo* messageBuffer);
  void onWP(const TMSPWP* messageBuffer);

  void teleport();

  void loadConfig(mINI::INIStructure& ini);
  void saveConfig(mINI::INIStructure& ini);

private:

  typedef struct
  {
    float lat;
    float lon;
    uint8_t flags;  //1-action waypoint
  } TCoords;

  XPLMMapLayerID layer = NULL;

  TMapMarkType markingType = MMT_LAT_LON_OSD;

  TCoords coords[MAX_MAP_POINTS];
  int pHead = 0;
  int pTail = 0;

  float crossLat = 0;
  float crossLon = 0;

  TCoords waypoints[MAX_WAYPOINTS];
  int waypointsCount = 0;

  int waypointsDownloadState = -1;
                                    
  void createOurMapLayer(const char * mapIdentifier, void * refcon);
  static void createOurMapLayerStatic(const char * mapIdentifier, void * refcon);
  static void willBeDeletedStatic(XPLMMapLayerID layer, void * inRefcon);
  void drawMarkings(XPLMMapLayerID layer, const float * inMapBoundsLeftTopRightBottom, float zoomRatio, float mapUnitsPerUserInterfaceUnit, XPLMMapStyle mapStyle, XPLMMapProjectionID projection, void * inRefcon);
  static void drawMarkingsStatic(XPLMMapLayerID layer, const float * inMapBoundsLeftTopRightBottom, float zoomRatio, float mapUnitsPerUserInterfaceUnit, XPLMMapStyle mapStyle, XPLMMapProjectionID projection, void * inRefcon);

  void addPoint(float lat, float lon);
  void addPointEx(float lat, float lon);

  void retrieveNextWaypoint();
};

extern TMap g_map;
