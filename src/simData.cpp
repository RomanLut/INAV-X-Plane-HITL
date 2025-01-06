#include "simData.h"
#include "stats.h"
#include "graph.h"
#include "util.h"
#include "osd.h"
#include "map.h"

#include <math.h>

TSimData g_simData;

#define GPS_RATE_HZ   5

#define DEG2RAD         (.017453292519943f) /* PI / 180 */
#define DECIDEGREES_TO_RADIANS(angle) (((angle) / 10.0f) * DEG2RAD)

//==============================================================
//==============================================================
void TSimData::init()
{
  this->gps_lastUpdate = GetTicks() - 1000;
  this->GPSHasNewData = false;

  this->gps_fix = GPS_FIX_3D;
  this->gps_numSat = 12;
  this->gps_glitch = GPS_GLITCH_NONE;
  this->gps_timeout = false;

  this->simulate_mag_failure = false;

  this->df_lattitude = XPLMFindDataRef("sim/flightmodel/position/latitude");
  this->df_longitude = XPLMFindDataRef("sim/flightmodel/position/longitude");
  this->df_elevation = XPLMFindDataRef("sim/flightmodel/position/elevation");
  //this->df_agl = XPLMFindDataRef("sim/flightmodel/position/y_agl");

  this->df_local_vx = XPLMFindDataRef("sim/flightmodel/position/local_vx");
  this->df_local_vy = XPLMFindDataRef("sim/flightmodel/position/local_vy");
  this->df_local_vz = XPLMFindDataRef("sim/flightmodel/position/local_vz");

  this->df_speed = XPLMFindDataRef("sim/flightmodel/position/groundspeed");
  this->df_airspeed = XPLMFindDataRef("sim/flightmodel/position/true_airspeed");

  this->df_roll = XPLMFindDataRef("sim/flightmodel/position/phi");
  this->df_pitch = XPLMFindDataRef("sim/flightmodel/position/theta");
  this->df_yaw = XPLMFindDataRef("sim/flightmodel/position/psi");
  this->df_hpath = XPLMFindDataRef("sim/flightmodel/position/hpath");

	// Accelerometer
  this->df_accel_x = XPLMFindDataRef("sim/flightmodel/forces/g_axil");
  this->df_accel_y = XPLMFindDataRef("sim/flightmodel/forces/g_side");
  this->df_accel_z = XPLMFindDataRef("sim/flightmodel/forces/g_nrml");

	// Gyro
  this->df_gyro_x = XPLMFindDataRef("sim/flightmodel/position/P");
  this->df_gyro_y = XPLMFindDataRef("sim/flightmodel/position/Q");
  this->df_gyro_z = XPLMFindDataRef("sim/flightmodel/position/R");

	// Barometer
  this->df_baro = XPLMFindDataRef("sim/weather/barometer_current_inhg");

	// // Velocity
	// xPlane.requestDataRef('sim/flightmodel/forces/vx_acf_axis', 10, function (ref, value) {
	//     simData.velocity_x = value; isSimDataUpdated = true;
	// });
	// xPlane.requestDataRef('sim/flightmodel/forces/vy_acf_axis', 10, function (ref, value) {
	//     simData.velocity_y = value; isSimDataUpdated = true;
	// });
	// xPlane.requestDataRef('sim/flightmodel/forces/vz_acf_axis', 10, function (ref, value) {
	//     simData.velocity_z = value; isSimDataUpdated = true;
	// });
	// xPlane.setDataRef('sim/operation/override/override_control_surfaces', 1);

	//static XPLMDataRef wrt = XPLMFindDataRef("sim/graphics/view/world_render_type");

  this->estimated_attitude_roll = 0;
  this->estimated_attitude_pitch = 0;
  this->estimated_attitude_yaw = 0;

  this->simulatePitot = true;
  this->simulatePitotFailureHW = false;
  this->simulatePitotFailure60 = false;
  this->airspeed = 0;

	//---- output ----
  this->muteBeeper = true;
  this->attitude_use_sensors = false;

  this->lastUpdateMS = 0;

  this->df_override_joystick = XPLMFindDataRef("sim/operation/override/override_joystick");
  this->df_control_throttle = XPLMFindDataRef("sim/cockpit2/engine/actuators/throttle_ratio_all");
  this->df_control_roll = XPLMFindDataRef("sim/joystick/yoke_roll_ratio");
  this->df_control_pitch = XPLMFindDataRef("sim/joystick/yoke_pitch_ratio");
  this->df_control_yaw = XPLMFindDataRef("sim/joystick/yoke_heading_ratio");

  this->isAirplane = false;
  this->isArmed = false;
  this->isOSDDisabled = false;
  this->isSupportedOSDNotFound = false;

  this->setBateryEmulation(BATTERY_INFINITE);
}

//==============================================================
//==============================================================
void TSimData::updateFromXPlane()
{
  uint32_t t = GetTicks();

  if ((t - this->gps_lastUpdate) >= (1000 / GPS_RATE_HZ) )                
  {
    this->gps_lastUpdate = t;
    this->GPSHasNewData = true;

    this->lattitude = XPLMGetDatad(this->df_lattitude);
    this->longitude = XPLMGetDatad(this->df_longitude);
    this->elevation = XPLMGetDatad(this->df_elevation);
    //this->agl = XPLMGetDataf(this->df_agl);

    this->local_vx = XPLMGetDataf(this->df_local_vx);
    this->local_vy = XPLMGetDataf(this->df_local_vy);
    this->local_vz = XPLMGetDataf(this->df_local_vz);

    this->speed = XPLMGetDataf(this->df_speed);
    this->airspeed = XPLMGetDataf(this->df_airspeed);
    this->course = XPLMGetDataf(this->df_hpath);
  }
  
  this->roll = XPLMGetDataf(this->df_roll);
  this->pitch = XPLMGetDataf(this->df_pitch);
  this->yaw = XPLMGetDataf(this->df_yaw);

  float kick = 0;
  if (this->autolaunch_kickStart != 0)
  {
    uint32_t t = GetTicks();
    uint32_t dt = t - this->autolaunch_kickStart;
    if (dt > 1000)
    {
      this->autolaunch_kickStart = 0;
    }
    else
    {
      kick = 4 * sin(dt / 180.0f * 3.14f);
    }
  }
  this->accel_x = XPLMGetDataf(this->df_accel_x) + kick;
  this->accel_y = XPLMGetDataf(this->df_accel_y);
  this->accel_z = XPLMGetDataf(this->df_accel_z);

  this->gyro_x = XPLMGetDataf(this->df_gyro_x);
  this->gyro_y = XPLMGetDataf(this->df_gyro_y);
  this->gyro_z = XPLMGetDataf(this->df_gyro_z);

  this->baro = XPLMGetDataf(this->df_baro);
}

//==============================================================
//==============================================================
void TSimData::sendToXPlane()
{
  XPLMSetDatai(this->df_override_joystick, 1);
	XPLMSetDataf(this->df_control_throttle, (this->control_throttle + 500.0f )/ 1000.0f);
	XPLMSetDataf(this->df_control_roll, this->control_roll / 500.0f);
	XPLMSetDataf(this->df_control_pitch, -(this->control_pitch / 500.0f ));
	XPLMSetDataf(this->df_control_yaw, -(this->control_yaw / 500.0f ));
}

//==============================================================
//==============================================================
void TSimData::updateFromINAV(const TMSPSimulatorFromINAV* data)
{
  this->control_throttle = data->throttle;
  this->control_roll = data->roll;
  this->control_pitch = data->pitch;
  this->control_yaw = data->yaw;

  this->isAirplane = (data->debugIndex & FIF_IS_AIRPLANE) != 0;
  bool prevArmed = this->isArmed;
  this->isArmed = (data->debugIndex & FIF_ARMED) != 0;
  this->isOSDDisabled = (data->debugIndex & FIF_OSD_DISABLED) != 0;
  this->isSupportedOSDNotFound = (data->debugIndex & FIF_ANALOG_OSD_NOT_FOUND) != 0;

  if (this->isArmed && !prevArmed)
  {
    disableBrakes();
    g_osd.setHome();
  }

  g_stats.debug[data->debugIndex & 7] = data->debugValue;

  this->estimated_attitude_roll = (int16_t)data->estimated_attitude_roll;
  this->estimated_attitude_pitch = (int16_t)data->estimated_attitude_pitch;
  this->estimated_attitude_yaw = (uint16_t)data->estimated_attitude_yaw;
  g_graph.addEstimatedAttitudeYPR(estimated_attitude_yaw, estimated_attitude_pitch, estimated_attitude_roll);

  g_graph.addDebug(data->debugIndex & 7, (float)data->debugValue);
  g_graph.addOutputYPR(this->control_yaw, this->control_pitch, this->control_roll);

  if ((data->debugIndex & 7 ) == 7)
  {
    g_map.addDebug(g_stats.debug[0], g_stats.debug[1]);
  }

  uint32_t t = GetTicks();
  uint32_t delta = t - this->lastUpdateMS;
  if ((this->lastUpdateMS != 0) && (delta < 300))
  {
    g_graph.addUpdatePeriodMS(delta);
  }
  this->lastUpdateMS = t;
}

//==============================================================
//==============================================================
void TSimData::sendToINAV()
{
  TMSPSimulatorToINAV data;

  data.version = MSP_SIMULATOR_VERSION;

  data.flags = SIMU_ENABLE |
    ((this->batEmulation != BATTERY_NONE) ? SIMU_SIMULATE_BATTERY : 0) |
    (this->muteBeeper ? SIMU_MUTE_BEEPER : 0) |
    (this->attitude_use_sensors ? SIMU_USE_SENSORS : 0) |
    (this->GPSHasNewData && !this->gps_timeout ? SIMU_HAS_NEW_GPS_DATA : 0) |
    SIMU_EXT_BATTERY_VOLTAGE |
    (this->simulatePitot ? SIMU_AIRSPEED : 0) |
    SIMU_EXTENDED_FLAGS;

  data.flags2 = (this->gps_timeout ? SIMU2_GPS_TIMEOUT >> 8 : 0) |
    (this->simulatePitotFailureHW ? SIMU2_PITOT_FAILURE >> 8 : 0);

  this->GPSHasNewData = false;

  data.fix = this->gps_fix;
  data.numSat = (uint8_t)this->gps_numSat;
  data.lat = (int32_t)round(this->lattitude * 10000000);
  data.lon = (int32_t)round(this->longitude * 10000000);      
  data.alt = (int32_t)round(this->elevation * 100); //expected by inav: elevation in cm
  data.speed = (int16_t)round(this->speed * 100); //expected by inav: ground speed cm/sec   
  data.airspeed = (uint16_t)round(this->airspeed * 100); //expected by inav: ground speed cm/sec
  if (this->simulatePitotFailure60)
  {
    data.airspeed = (uint16_t)round(1777); 
  }
  data.course = (int16_t)round(this->course * 10);  // expected by inav: deg * 10
  if (data.course < 0) data.course += 3600;

  data.velNED[0] = (int16_t)round(-this->local_vz*100); // nedVelNorth;  
  data.velNED[1] = (int16_t)round( this->local_vx*100); //nedVelEast
  data.velNED[2] = (int16_t)round(-this->local_vy*100); //nedVelDown

  if (this->gps_glitch != GPS_GLITCH_NONE)
  {
    if (this->glitch_lattitude == 0)
    {
      this->glitch_lattitude = this->lattitude;
      this->glitch_longitude = this->longitude;
      this->glitch_elevation = this->elevation;
    }
  }
  else
  {
    this->glitch_lattitude = 0;
    this->glitch_longitude = 0;
    this->glitch_elevation = 0;
  }

  if (this->gps_glitch == GPS_GLITCH_FREEZE)
  {
    data.lat = (int32_t)round(this->glitch_lattitude * 10000000);
    data.lon = (int32_t)round(this->glitch_longitude * 10000000);
    data.alt = (int32_t)round(this->glitch_elevation * 100);
    data.speed = 0;
    data.velNED[0] = 0;
    data.velNED[1] = 0;
    data.velNED[2] = 0;
  }
  else if (this->gps_glitch == GPS_GLITCH_OFFSET)
  {
    data.lat = (int32_t)round((this->lattitude + 5 / 111.32) * 10000000);
    data.lon = (int32_t)round(this->longitude * 10000000);
    data.alt = (int32_t)round(this->elevation * 100);
  }
  else if (this->gps_glitch == GPS_GLITCH_LINEAR)
  {
    float k = GetTicks() / 100000.0f;
    k -= (int)k;
    data.lat = (int32_t)round((this->lattitude + 1 * k / 111.32) * 10000000);
    data.lon = (int32_t)round(this->longitude * 10000000);
    data.alt = (int32_t)round(this->elevation * 100);
  }
  else if (this->gps_glitch == GPS_GLITCH_ALTITUDE)
  {
    float k = GetTicks() / 100000.0f;
    k -= (int)k;
    data.alt = (int32_t)round(this->glitch_elevation * 100 + k * 100000);
    data.velNED[2] = -(int16_t)round(k * 100000);
  }
  else if ((this->gps_glitch == GPS_GLITCH_CIRCLE) || (this->gps_glitch == GPS_GLITCH_CIRCLE_ALTITUDE) || (this->gps_glitch == GPS_GLITCH_CIRCLE_ALTITUDE_5))
  {
    float k = GetTicks() / 100000.0f;
    k -= (int)k;
    double a = k * 2 * CONST_PI;
    data.lat = (int32_t)round((this->glitch_lattitude + 1 * sin(a) / 111.32) * 10000000);
    data.lon = (int32_t)round((this->glitch_longitude + 1 * cos(a) / 111.32) * 10000000);
    data.alt = (int32_t)round(this->glitch_elevation * 100 + ((this->gps_glitch == GPS_GLITCH_CIRCLE_ALTITUDE) ? k * 100000 : 0));
    data.velNED[0] = (int16_t)round(sin(a + CONST_PI / 2) * 6283);
    data.velNED[1] = (int16_t)round(cos(a + CONST_PI / 2) * 6283);
    data.velNED[2] = (this->gps_glitch == GPS_GLITCH_CIRCLE_ALTITUDE) ? -(int16_t)round(k * 100000) : 0;
    data.course = (int16_t)round((360 - (a * 180 / CONST_PI)) * 10);  // expected by inav: deg * 10
    if (data.course > 3600) data.course -= 3600;
    data.speed = 6283;
  }

  //expected order of rotation from local to global: roll, pitch, yaw
  data.roll =  (int16_t)round( this->roll * 10);  //expected by inav: left wing down - negative roll, 1 degree = 10, values range: -1800...1800
  data.pitch = (int16_t)round(-this->pitch * 10);  //expected by inav: stick down/nose up - negative pitch, upside-down: stick down/nose up - positiive pitch, 1 degree = 10 , values range: -1800...1800
  data.yaw =   (int16_t)round( this->yaw * 10);  //expected by inav: rotate clockwise( top view) - positive yaw+, 1 degreee = 10 , values range: 0...3600 , north = 0
  if (data.yaw < 0) data.yaw += 3600;

  g_graph.addAttitudeYPR(data.yaw, data.pitch, data.roll);

  data.accel_x = clampToInt16( -this->accel_x * 1000 );  //expected by inav: forward - positive   
  data.accel_y = clampToInt16(  this->accel_y * 1000 );  //expected by inav: right - negative
  data.accel_z = clampToInt16(  this->accel_z * 1000 );  //expected by inav: 1.0f in stable position (1G)

  g_graph.addACC(data.accel_x / 1000.0f, data.accel_y / 1000.0f, data.accel_z / 1000.0f);

  g_stats.dbg_acc_x = data.accel_x / 1000.0f;
  g_stats.dbg_acc_y = data.accel_y / 1000.0f;              
  g_stats.dbg_acc_z = data.accel_z / 1000.0f;

  data.gyro_x = clampToInt16(  this->gyro_x * 16.0f );  //expected by inav: roll left wing down rotation -> negative
  data.gyro_y = clampToInt16( -this->gyro_y * 16.0f );  //expected by inav: pitch up rotation -> negative, 1 deerees per second
  data.gyro_z = clampToInt16( -this->gyro_z * 16.0f );  //expected by inav: yaw clockwise rotation (top view) ->negative

  g_graph.addGyro(data.gyro_x / 16.0f, data.gyro_y / 16.0f, data.gyro_z / 16.0f);

  g_stats.dbg_gyro_x = data.gyro_x / 16.0f;  
  g_stats.dbg_gyro_y = data.gyro_y / 16.0f;  
  g_stats.dbg_gyro_z = data.gyro_z / 16.0f;  

  data.baro = (int32_t)round(this->baro * 3386.39f);

  float north[3];
  north[0] = 1.0f;
  north[1] = 0;
  north[2] = 0;

  float quat[4];
  this->computeQuaternionFromRPY(quat, data.roll, data.pitch, data.yaw );
  this->transformVectorEarthToBody(north, quat);

  if (!this->simulate_mag_failure)
  {
    data.mag_x = clampToInt16(north[0] * 16000.0f);
    data.mag_y = clampToInt16(north[1] * 16000.0f);
    data.mag_z = clampToInt16(north[2] * 16000.0f);
  }
  else
  {
    data.mag_x = 0;
    data.mag_y = 0;
    data.mag_z = 0;
  }

  this->recalculateBattery();
  data.vbat = (uint8_t)round(this->battery_chargeV * 10);

  g_msp.sendCommand(MSP_SIMULATOR, &data, sizeof(data));
}

//==============================================================
//==============================================================
void TSimData::computeQuaternionFromRPY( float* quat, int16_t roll, int16_t pitch, int16_t yaw)
{
  if (roll > 1800) roll -= 3600;
  if (pitch > 1800) pitch -= 3600;
  if (yaw > 1800) yaw -= 3600;

  const float cosRoll = cosf(DECIDEGREES_TO_RADIANS(roll) * 0.5f);
  const float sinRoll = sinf(DECIDEGREES_TO_RADIANS(roll) * 0.5f);

  const float cosPitch = cosf(DECIDEGREES_TO_RADIANS(pitch) * 0.5f);
  const float sinPitch = sinf(DECIDEGREES_TO_RADIANS(pitch) * 0.5f);

  const float cosYaw = cosf(DECIDEGREES_TO_RADIANS(-yaw) * 0.5f);
  const float sinYaw = sinf(DECIDEGREES_TO_RADIANS(-yaw) * 0.5f);

  quat[0] = cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw;
  quat[1] = sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw;
  quat[2] = cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw;
  quat[3] = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;
}

//==============================================================
//==============================================================
void TSimData::transformVectorEarthToBody(float* v, float* quat)
{
  // HACK: This is needed to correctly transform from NED (sensor frame) to NEU (navigation)
  v[1] = -v[1];

  // From earth frame to body frame
  this->quaternionRotateVector(v, v, quat);
}

//==============================================================
//==============================================================
void TSimData::quaternionRotateVector(float* result, const float* vect, const float* quat)
{
  float vectQuat[4];
  float refConj[4];

  vectQuat[0] = 0;
  vectQuat[1] = vect[0];
  vectQuat[2] = vect[1];
  vectQuat[3] = vect[2];

  this->quaternionConjugate(refConj, quat);
  this->quaternionMultiply(vectQuat, refConj, vectQuat);
  this->quaternionMultiply(vectQuat, vectQuat, quat);

  result[0] = vectQuat[1];
  result[1] = vectQuat[2];
  result[2] = vectQuat[3];
}

//==============================================================
//==============================================================
void TSimData::quaternionConjugate(float* result, const float* q)
{
  result[0] = q[0];
  result[1] = -q[1];
  result[2] = -q[2];
  result[3] = -q[3];
}

//==============================================================
//==============================================================
void TSimData::quaternionMultiply(float* result, const float* a, const float* b)
{
  float p[4];

  p[0] = a[0] * b[0] - a[1] * b[1] - a[2] * b[2] - a[3] * b[3];
  p[1] = a[0] * b[1] + a[1] * b[0] + a[2] * b[3] - a[3] * b[2];
  p[2] = a[0] * b[2] - a[1] * b[3] + a[2] * b[0] + a[3] * b[1];
  p[3] = a[0] * b[3] + a[1] * b[2] - a[2] * b[1] + a[3] * b[0];

  result[0] = p[0];
  result[1] = p[1];
  result[2] = p[2];
  result[3] = p[3];
}

//==============================================================
//==============================================================
void TSimData::disconnect()
{
  TMSPSimulatorToINAV data;
  data.version = 1;
  data.flags = 0;
  g_msp.sendCommand(MSP_SIMULATOR, &data, sizeof(data));

  this->control_throttle = -500;
  this->control_roll = 0;
  this->control_pitch = 0;
  this->control_yaw = 0;

  this->sendToXPlane();
}

//==============================================================
//==============================================================
void TSimData::setBateryEmulation(TBatteryEmulationType s)
{
  this->batEmulation = s;
  this->battery_lastUpdate = GetTicks();
  this->battery_chargeV = (s == BATTERY_DISCHAGED) ? 9.8f : 12.6f;
}

//==============================================================
//==============================================================
void TSimData::recalculateBattery()
{
  uint32_t t = GetTicks();
  uint32_t dt = t - this->battery_lastUpdate;
  this->battery_lastUpdate = t;
  if (dt > 1000) dt = 1000;

  float k = (this->control_throttle + 500.0f) / 1000.0f;
  if (k < 0) k = 0;
  if (k > 1.0f) k = 1.0f;

  //k=k*k;
  //use linear dependency for simplicity

  //12.6-9.6 = 3V
  float n = 0;
  switch (this->batEmulation)
  {
  case BATTERY_3MIN:
    n = 3.0f;
    break;
  case BATTERY_10MIN:
    n = 10.0f;
    break;
  case BATTERY_30MIN:
    n = 30.0f;
    break;
  }

  if (n > 0)
  {
    this->battery_chargeV -= k * dt * 3.0f / (1000 * 60 * n);
  }

  if (this->battery_chargeV < 9.6f)
  {
    this->battery_chargeV = 9.6f;
  }
}

//==============================================================
//==============================================================
void TSimData::loadConfig(mINI::INIStructure& ini)
{
  if (ini[SETTINGS_SECTION].has(SETTINGS_GPS_NUMSAT))
  {
    this->gps_numSat = atoi( ini[SETTINGS_SECTION][SETTINGS_GPS_NUMSAT].c_str() );
    if (this->gps_numSat == 0)
    {
      this->gps_fix = GPS_NO_FIX;
    }
    else if (gps_numSat == 12)
    {
      this->gps_fix = GPS_FIX_3D;
    }
    else
    {
      this->gps_fix = GPS_FIX_3D;
      this->gps_numSat = 12;
    }
  }

  this->gps_timeout = ini[SETTINGS_SECTION].has(SETTINGS_GPS_TIMEOUT) && (ini[SETTINGS_SECTION][SETTINGS_GPS_TIMEOUT] == "1");
  this->gps_glitch = GPS_GLITCH_NONE;
  if (ini[SETTINGS_SECTION].has(SETTINGS_GPS_GLITCH))
  {
    this->gps_glitch = atoi(ini[SETTINGS_SECTION][SETTINGS_GPS_GLITCH].c_str());
  }

  this->simulate_mag_failure = ini[SETTINGS_SECTION].has(SETTINGS_MAG_FAILURE) && (ini[SETTINGS_SECTION][SETTINGS_MAG_FAILURE] == "1");

  this->attitude_use_sensors = ini[SETTINGS_SECTION].has(SETTINGS_ATTITUDE_USE_SENSORS) && (ini[SETTINGS_SECTION][SETTINGS_ATTITUDE_USE_SENSORS] == "1");

  if (ini[SETTINGS_SECTION].has(SETTINGS_BATTERY_EMULATION))
  {
    this->batEmulation = (TBatteryEmulationType)atoi(ini[SETTINGS_SECTION][SETTINGS_BATTERY_EMULATION].c_str());
    if (this->batEmulation < BATTERY_NONE || this->batEmulation > BATTERY_30MIN)
    {
      this->batEmulation = BATTERY_INFINITE;
    }
  }

  this->muteBeeper = ini[SETTINGS_SECTION].has(SETTINGS_MUTE_BEEPER) && (ini[SETTINGS_SECTION][SETTINGS_MUTE_BEEPER] != "0");

  this->simulatePitot = !ini[SETTINGS_SECTION].has(SETTINGS_SIMULATE_PITOT) || (ini[SETTINGS_SECTION][SETTINGS_SIMULATE_PITOT] != "0");
  this->simulatePitotFailureHW = ini[SETTINGS_SECTION].has(SETTINGS_SIMULATE_PITOT_FAILURE_HW) && (ini[SETTINGS_SECTION][SETTINGS_SIMULATE_PITOT_FAILURE_HW] == "1");
  this->simulatePitotFailure60 = ini[SETTINGS_SECTION].has(SETTINGS_SIMULATE_PITOT_FAILURE_60) && (ini[SETTINGS_SECTION][SETTINGS_SIMULATE_PITOT_FAILURE_60] == "1");
  if (this->simulatePitot)
  {
    this->simulatePitotFailureHW = false;
    this->simulatePitotFailure60 = false;
  }
  if (this->simulatePitotFailureHW) this->simulatePitotFailure60 = false;
}

//==============================================================
//==============================================================
void TSimData::saveConfig(mINI::INIStructure& ini)
{
  ini[SETTINGS_SECTION][SETTINGS_GPS_NUMSAT] = std::to_string(this->gps_numSat);
  ini[SETTINGS_SECTION][SETTINGS_ATTITUDE_USE_SENSORS] = std::to_string(this->attitude_use_sensors ? 1 : 0);
  ini[SETTINGS_SECTION][SETTINGS_GPS_TIMEOUT] = std::to_string(this->gps_timeout ? 1 : 0);
  ini[SETTINGS_SECTION][SETTINGS_GPS_GLITCH] = std::to_string(this->gps_glitch);
  ini[SETTINGS_SECTION][SETTINGS_MAG_FAILURE] = std::to_string(this->simulate_mag_failure ? 1 : 0);
  ini[SETTINGS_SECTION][SETTINGS_BATTERY_EMULATION] = std::to_string(this->batEmulation);
  ini[SETTINGS_SECTION][SETTINGS_MUTE_BEEPER] = std::to_string(this->muteBeeper ? 1 : 0);
  ini[SETTINGS_SECTION][SETTINGS_SIMULATE_PITOT] = std::to_string(this->simulatePitot ? 1 : 0);
  ini[SETTINGS_SECTION][SETTINGS_SIMULATE_PITOT_FAILURE_HW] = std::to_string(this->simulatePitotFailureHW ? 1 : 0);
  ini[SETTINGS_SECTION][SETTINGS_SIMULATE_PITOT_FAILURE_60] = std::to_string(this->simulatePitotFailure60 ? 1 : 0);
}

