

#include "graph.h"
#include "util.h"

#include <math.h>

TGraph g_graph;



//=============================================================
//=============================================================
void TGraphSeries::setRange(float min, float max)
{
  this->min = min;
  this->max = max;
  this->autoRange =  (min == 0) && (max == 0);

  this->clear();
}

//==============================================================
//==============================================================
void TGraphSeries::clear()
{
  for (int i = 0; i < GRAPH_POINTS; i++)
  {
    this->points[i] = (this->min + this->max)/2.0f;
  }
  this->head = 0;
}

//==============================================================
//==============================================================
void TGraphSeries::setName(const char* pName)
{
  strcpy(this->name, pName);
}


//=============================================================
//=============================================================
void TGraphSeries::addPoint(float value)
{
  this->points[this->head++] = value;
  if (this->head == GRAPH_POINTS)
  {
    this->head = 0;
  }
}

//=============================================================
//=============================================================
void TGraphSeries::drawOSD(float bx, float by, float width, float height)
{
  if (this->min == this->max)
  {
    this->min = -1;
    this->max = 1;
  }

  glColor4f(((this->color >> 16) & 0xff) / 255.0f, ((this->color >> 8) & 0xff) / 255.0f, ((this->color >> 0) & 0xff) / 255.0f, 1.0f);
  glLineWidth(1.0f);

  glBegin(GL_LINE_STRIP);

  float r = this->max;
  bool adj = false;

  int head = this->head - 1;
  for (int i = 0; i < GRAPH_POINTS; i++)
  {
    if (head < 0) head = GRAPH_POINTS - 1;
    float v = this->points[head--];

    if (fabs(v) > r)
    {
      r = fabs(v);
      adj = true;
    }

    if (v < this->min) v = this->min;
    if (v > this->max) v = this->max;

    v = (v - this->min) / (this->max - this->min) * height;

    glVertex2f(bx + width - i * 2 - 1, by + v);           
  }

  glEnd();

  if (adj && this->autoRange)
  {
    r *= 1.1f;

    if (r < 10)
    {
      r = ceil(r);
    }
    else if (r < 100)
    {
      r = ceil(r / 10) * 10;
    }
    else if (r < 1000)
    {
      r = ceil(r / 100) * 100;
    }
    else
    {
      r = ceil(r / 1000) * 1000;
    }

    this->min = -r;
    this->max = r;
  }
}

//======================================================================================================================================
//======================================================================================================================================

//=============================================================
//=============================================================
TGraph::TGraph()
{
  this->series[0].color = (255ul << 16) + (255ul << 8 );
  this->series[1].color = 255ul << 8;
  this->series[2].color = 255ul << 16;
  this->series[3].color = (190ul << 16) + (190ul << 8 );
  this->series[4].color = 164ul << 8;
  this->series[5].color = 190ul << 16;
  this->series[6].color = (128ul << 8) + 255ul;
  this->series[7].color = (255ul << 8) + 255ul;

  this->activeCount = 0;
  this->lastLen = 0;
  this->pSeriesName = "";
  memset(this->debug, 0, sizeof(float) * 8);

  this->lastUpdatesCountTime = 0;
  this->updatesCount = 0;
  this->updatesCountValue = 0;

}

//==============================================================
//==============================================================
void TGraph::drawCallback()
{
  if (this->graph_type == GRAPH_NONE) return;

  int sx, sy;
  XPLMGetScreenSize(&sx, &sy);

  // The drawing part.
  XPLMSetGraphicsState(
    0,        // No fog, equivalent to glDisable(GL_FOG);
    0,        // One texture, equivalent to glEnable(GL_TEXTURE_2D);
    0,        // No lighting, equivalent to glDisable(GL_LIGHT0);
    0,        // No alpha testing, e.g glDisable(GL_ALPHA_TEST);
    1,        // Use alpha blending, e.g. glEnable(GL_BLEND);
    0,        // No depth read, e.g. glDisable(GL_DEPTH_TEST);
    0);        // No depth write, e.g. glDepthMask(GL_FALSE);


  float width = GRAPH_POINTS*2;
  float height = 100*4;
  float bx = sx - width - 20.0f;
  float by = sy - height - 20.0f;

  glColor4f(0, 0, 0, 0.5f);
  glBegin(GL_QUADS);
  glVertex2f(bx - 1, by - 1);
  glVertex2f(bx - 1, by + 1 + height);
  glVertex2f(bx + width, by + 1 + height);
  glVertex2f(bx + width, by - 1);
  glEnd();

  for (int i = this->activeCount-1; i >=0 ; i--)       
  {
    this->series[i].drawOSD(bx, by, width, height);
  }

  float lineHeight = 16.0f;

  float y = by - 4 + height - lineHeight;                                             
  float col[] = { 1, 1, 1 };

  XPLMDrawString(col, (int)(bx + 4.0f), (int)y, (char*)this->pSeriesName, NULL, xplmFont_Basic);

  float y2 = by + 4 + this->activeCount * lineHeight;

  for (int i = 0; i < this->activeCount; i++)
  {
    char msg[100];
    char msg1[100];
    char msg2[100];

    float col[] = { ((series[i].color >> 16)&0xff)/255.0f, ((series[i].color >> 8) & 0xff) / 255.0f,((series[i].color >> 0) & 0xff) / 255.0f };

    this->formatRangeNumber(msg1, this->series[i].min);
    this->formatRangeNumber(msg2, this->series[i].max);
    sprintf(msg, "%s[%s...%s]", this->series[i].name, msg1, msg2);

    size_t l = strlen(msg);
    if (this->lastLen < l) this->lastLen = l;

    while (l < (this->lastLen + 2))
    {
      msg[l++] = '_';
    }
    msg[l] = 0;

    this->formatValueNumber(msg1, this->series[i].points[this->series[i].head == 0 ? GRAPH_POINTS-1 : this->series[i].head-1] );
    strcat(msg, msg1);
    XPLMDrawString(col, (int)(bx + 4.0f), (int)y2, msg, NULL, xplmFont_Basic);
    y2 -= lineHeight;
  }

  glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
  glLineWidth(1.0f);

  glBegin(GL_LINE_STRIP);
  glVertex2f(bx, by);
  glVertex2f(bx, by + 1 + height);
  glVertex2f(bx + width, by + 1 + height);
  glVertex2f(bx + width, by);
  glVertex2f(bx, by);
  glVertex2f(bx, by + height / 2.0f);
  glVertex2f(bx + width, by + height / 2.0f);
  glEnd();

}

//==============================================================
//==============================================================
void TGraph::setGraphType(TGraphType type)
{
  this->graph_type = type;
  this->clear();

  switch (this->graph_type)
  {
  case GRAPH_UPDATES:
    this->pSeriesName = "Updates period, MS";
    this->pSeriesName = "Updates per second";
    this->activeCount = 2;
    this->series[0].setRange(0, 0);
    this->series[1].setRange(0, 0);
    break;

  case GRAPH_ATTITUDE_OUTPUT:
    this->pSeriesName = "Attitude, output";
    this->activeCount = 6;

    this->series[0].setRange(0, 3600);
    this->series[1].setRange(-1800, 1800);
    this->series[2].setRange(-1800, 1800);
    this->series[3].setRange(-500, 500);
    this->series[4].setRange(-500, 500);
    this->series[5].setRange(-500, 500);
    this->series[0].setName("YAW__________");
    this->series[1].setName("PITCH________");
    this->series[2].setName("ROLL_________");
    this->series[3].setName("Output YAW___");
    this->series[4].setName("Output PITCH_");
    this->series[5].setName("Output ROLL__");
    break;

  case GRAPH_ATTITUDE_ESTIMATION:
    this->pSeriesName = "Attitude estimation";
    this->activeCount = 6;

    this->series[0].setRange(0, 3600);
    this->series[1].setRange(-1800, 1800);
    this->series[2].setRange(-1800, 1800);
    this->series[3].setRange(0, 3600);
    this->series[4].setRange(-1800, 1800);
    this->series[5].setRange(-1800, 1800);
    this->series[0].setName("Real YAW_________");
    this->series[1].setName("Real PITCH_______");
    this->series[2].setName("Real ROLL________");
    this->series[3].setName("Estimated YAW____");
    this->series[4].setName("Estimated PITCH__");
    this->series[5].setName("Estimated ROLL___");
    break;

  case GRAPH_ACC:
    this->pSeriesName = "Accelerometer";
    this->activeCount = 3;
    this->series[0].setRange(-8, 8);
    this->series[1].setRange(-8, 8);
    this->series[2].setRange(-8, 8);
    this->series[0].setName("X ");
    this->series[1].setName("Y ");
    this->series[2].setName("Z ");
    break;

  case GRAPH_GYRO:
    this->pSeriesName = "Gyroscope";
    this->activeCount = 3;
    this->series[0].setRange(-64, 64);
    this->series[1].setRange(-64, 64);
    this->series[2].setRange(-64, 64);
    this->series[0].setName("X ");
    this->series[1].setName("Y ");
    this->series[2].setName("Z ");
    break;

  case GRAPH_DEBUG_ALTITUDE:
    this->pSeriesName = "debug_mode = altitude";
    this->activeCount = 8;
    this->series[0].setRange(0, 0);
    this->series[1].setRange(0, 0);
    this->series[2].setRange(0, 0);
    this->series[3].setRange(0, 0);
    this->series[4].setRange(0, 0);
    this->series[5].setRange(0, 0);
    this->series[6].setRange(0, 0);
    this->series[7].setRange(0, 0);

    this->series[0].setName("posEstimator.est.pos.z______");
    this->series[1].setName("posEstimator.est.vel.z______");
    this->series[2].setName("imuMeasuredAccelBF.z        ");
    this->series[3].setName("posEstimator.imu.accelNEU.z_");
    this->series[4].setName("posEstimator.gps.pos.z______");
    this->series[5].setName("posEstimator.gps.vel.z______");
    this->series[6].setName("accGetVibrationLevel()______");
    this->series[7].setName("accGetClipCount()___________");
    break;

  case GRAPH_DEBUG_CUSTOM:
    this->pSeriesName = "debug[8]";
    this->activeCount = 8;
    this->series[0].setRange(0, 0);
    this->series[1].setRange(0, 0);
    this->series[2].setRange(0, 0);
    this->series[3].setRange(0, 0);
    this->series[4].setRange(0, 0);
    this->series[5].setRange(0, 0);
    this->series[6].setRange(0, 0);
    this->series[7].setRange(0, 0);

    this->series[0].setName("debug[0] ");
    this->series[1].setName("debug[1] ");
    this->series[2].setName("debug[2] ");
    this->series[3].setName("debug[3] ");
    this->series[4].setName("debug[4] ");
    this->series[5].setName("debug[5] ");
    this->series[6].setName("debug[6] ");
    this->series[7].setName("debug[7] ");
    break;

  }
}

//==============================================================
//==============================================================
TGraphType TGraph::getGraphType()
{
  return this->graph_type;
}

//==============================================================
//==============================================================
void TGraph::clear()
{
  for (int i = 0; i < GRAPH_COUNT_MAX; i++)
  {
    this->series[i].clear();
  }
  this->lastLen = 0;
}

//==============================================================
//==============================================================
void TGraph::addOutputYPR(float yaw, float pitch, float roll)
{
  if (this->graph_type != GRAPH_ATTITUDE_OUTPUT) return;
  this->series[3].addPoint(yaw);
  this->series[4].addPoint(pitch);
  this->series[5].addPoint(roll);
}

//==============================================================
//==============================================================
void TGraph::addAttitudeYPR(float yaw, float pitch, float roll)
{
  if ((this->graph_type != GRAPH_ATTITUDE_OUTPUT) && (this->graph_type != GRAPH_ATTITUDE_ESTIMATION)) return;
  this->series[0].addPoint(yaw);
  this->series[1].addPoint(pitch);
  this->series[2].addPoint(roll);
}

//==============================================================
//==============================================================
void TGraph::addEstimatedAttitudeYPR(float yaw, float pitch, float roll)
{
  if (this->graph_type != GRAPH_ATTITUDE_ESTIMATION) return;
  this->series[3].addPoint(yaw);
  this->series[4].addPoint(pitch);
  this->series[5].addPoint(roll);
}

//==============================================================
//==============================================================
void TGraph::addACC(float x, float y, float z)
{
  if (this->graph_type != GRAPH_ACC) return;
  this->series[0].addPoint(x);
  this->series[1].addPoint(y);
  this->series[2].addPoint(z);
}

//==============================================================
//==============================================================
void TGraph::addGyro(float x, float y, float z)
{
  if (this->graph_type != GRAPH_GYRO) return;
  this->series[0].addPoint(x);
  this->series[1].addPoint(y);
  this->series[2].addPoint(z);
}

//==============================================================
//==============================================================
void TGraph::addDebug(int index, float value)
{
  if ((this->graph_type != GRAPH_DEBUG_ALTITUDE) && (this->graph_type != GRAPH_DEBUG_CUSTOM)) return;

  this->debug[index] = value;
  if (index == 7)
  {
    for (int i = 0; i < activeCount; i++)
    {
      this->series[i].addPoint(this->debug[i]);
    }
  }
}

//==============================================================
//==============================================================
void TGraph::addUpdatePeriodMS(uint32_t period)
{
  if (this->graph_type != GRAPH_UPDATES) return;
  this->series[0].addPoint((float)period);

  this->updatesCount++;
  uint32_t t = GetTicks();
  uint32_t delta = t - this->lastUpdatesCountTime;
  if (delta >= 1000)
  {
    this->updatesCountValue = this->updatesCount;
    this->updatesCount = 0;
    this->lastUpdatesCountTime = t;
  }

  this->series[1].addPoint((float)this->updatesCountValue);
}

//==============================================================
//==============================================================
void TGraph::formatRangeNumber(char* dest, float value)
{
  if (value == 0)
  {
    strcpy(dest, "0");
  }
  else if (fabs(value) > 64 )
  {
    sprintf(dest, "%+1.0f", value);
  }
  else if (fabs(value) > 8)
  {
    sprintf(dest, "%+1.1f", value);
  }
  else
  {
    sprintf(dest, "%+1.1f", value);
  }
}

//==============================================================
//==============================================================
void TGraph::formatValueNumber(char* dest, float value)
{
  if (fabs(value) > 1000)
  {
    sprintf(dest, "%+1.0f", value);
  }
  else if (fabs(value) > 64)
  {
    sprintf(dest, "%+1.1f", value);
  }
  else if (fabs(value) > 8)
  {
    sprintf(dest, "%+1.2f", value);
  }
  else
  {
    sprintf(dest, "%+1.3f", value);
  }
}

//==============================================================
//==============================================================
void TGraph::loadConfig(mINI::INIStructure& ini)
{
  if (ini[SETTINGS_SECTION].has(SETTINGS_GRAPH_TYPE))
  {
    TGraphType val = (TGraphType)atoi(ini[SETTINGS_SECTION][SETTINGS_GRAPH_TYPE].c_str());
    if (val < GRAPH_NONE || val > GRAPH_DEBUG_CUSTOM)
    {
      val = GRAPH_NONE;
    }
    this->setGraphType(val);
  }
}

//==============================================================
//==============================================================
void TGraph::saveConfig(mINI::INIStructure& ini)
{
  ini[SETTINGS_SECTION][SETTINGS_GRAPH_TYPE] = std::to_string(this->getGraphType());
}

