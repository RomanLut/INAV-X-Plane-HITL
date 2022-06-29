#pragma once

#include "config.h"

typedef enum
{
  GRAPH_NONE,
  GRAPH_ATTITUDE_RPY,
  GRAPH_ACC,
  GRAPH_GYRO,
  GRAPH_DEBUG_ALTITUDE
} TGraphType;

#define GRAPH_POINTS      200
#define GRAPH_COUNT_MAX  10

//=======================================================
//=======================================================
class TGraphSeries
{
  public:

    float points[GRAPH_POINTS];
    float max;
    float min;

    bool autoRange;

    int color;

    int head;

    char name[128];

    void clear();

    void setRange(float min, float max);
    void setName(const char* pName);

    void addPoint(float value);

    void draw(float bx, float by, float width, float height);
};


//=======================================================
//=======================================================
class TGraph
{
public:
  TGraphSeries series[GRAPH_COUNT_MAX];

  TGraph();

  void drawCallback();

  void setGraphType(TGraphType type);
  TGraphType getGraphType();

  void clear();

  void addOutputYPR(float yaw, float pitch, float roll);
  void addAttitudeYPR(float yaw, float pitch, float roll);
  void addACC(float x, float y, float z );
  void addGyro(float x, float y, float z);

  void addDebug(int index, float value);

private:

  TGraphType graph_type = GRAPH_NONE;

  int activeCount;
  const char* pSeriesName;

  float debug[8];

  void formatRangeNumber(char* dest, float value);
  void formatValueNumber(char* dest, float value);
};

extern TGraph g_graph;
