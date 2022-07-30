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
  this->gps_lastUpdate = GetTickCount() - 1000;
  this->GPSHasNewData = false;

  this->gps_fix = GPS_FIX_3D;
  this->gps_numSat = 12;
  this->gps_spoofing = 0;

  this->df_lattitude = XPLMFindDataRef("sim/flightmodel/position/latitude");
  this->df_longitude = XPLMFindDataRef("sim/flightmodel/position/longitude");
  this->df_elevation = XPLMFindDataRef("sim/flightmodel/position/elevation");
  //this->df_agl = XPLMFindDataRef("sim/flightmodel/position/y_agl");

  this->df_local_vx = XPLMFindDataRef("sim/flightmodel/position/local_vx");
  this->df_local_vy = XPLMFindDataRef("sim/flightmodel/position/local_vy");
  this->df_local_vz = XPLMFindDataRef("sim/flightmodel/position/local_vz");

  this->df_speed = XPLMFindDataRef("sim/flightmodel/position/groundspeed");

  this->df_roll = XPLMFindDataRef("sim/flightmodel/position/phi");
  this->df_pitch = XPLMFindDataRef("sim/flightmodel/position/theta");
  this->df_yaw = XPLMFindDataRef("sim/flightmodel/position/psi");

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
  this->isOSDAnalogOSDNotFound = false;

  this->setBateryEmulation(BATTERY_INFINITE);
}

//==============================================================
//==============================================================
void TSimData::updateFromXPlane()
{
  uint32_t t = GetTickCount();

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
    this->course = XPLMGetDataf(this->df_yaw);
  }
  
  this->roll = XPLMGetDataf(this->df_roll);
  this->pitch = XPLMGetDataf(this->df_pitch);
  this->yaw = XPLMGetDataf(this->df_yaw);

  this->accel_x = XPLMGetDataf(this->df_accel_x);
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
  this->isOSDAnalogOSDNotFound = (data->debugIndex & FIF_ANALOG_OSD_NOT_FOUND) != 0;

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

  uint32_t t = GetTickCount();
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
    ((this->batEmulation != BATTERY_NONE)? SIMU_SIMULATE_BATTERY : 0) |
    (this->muteBeeper ? SIMU_MUTE_BEEPER : 0) |
    (this->attitude_use_sensors ? SIMU_USE_SENSORS : 0) |
    (this->GPSHasNewData ? SIMU_HAS_NEW_GPS_DATA : 0 ) |
    SIMU_EXT_BATTERY_VOLTAGE;

  this->GPSHasNewData = false;

  data.fix = this->gps_fix;
  data.numSat = (uint8_t)this->gps_numSat;
  data.lat = (int32_t)round(this->lattitude * 10000000);
  data.lon = (int32_t)round(this->longitude * 10000000);
  data.alt = (int32_t)round(this->elevation * 100); //expected by inav: elevation in cm
  data.speed = (int16_t)round(this->speed * 100); //expected by inav: ground speed cm/sec   
  data.course = (int16_t)round(this->course * 10);  // expected by inav: deg * 10
                                                                                      
  data.velNED[0] = (int16_t)round(-this->local_vz*100); // nedVelNorth;  
  data.velNED[1] = (int16_t)round( this->local_vx*100); //nedVelEast
  data.velNED[2] = (int16_t)round(-this->local_vy*100); //nedVelDown

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

  data.mag_x = clampToInt16(north[0] * 16000.0f);
  data.mag_y = clampToInt16(north[1] * 16000.0f);
  data.mag_z = clampToInt16(north[2] * 16000.0f);

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
  this->battery_lastUpdate = GetTickCount();
  this->battery_chargeV = (s == BATTERY_DISCHAGED) ? 9.8f : 12.6f;
}

//==============================================================
//==============================================================
void TSimData::recalculateBattery()
{
  uint32_t t = GetTickCount();
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

