#pragma once

#include "config.h"

#include "msp.h"
#include "util.h"

#define GPS_NO_FIX  0
#define GPS_FIX_2D  1
#define GPS_FIX_3D  2

#define DEBUG_U32_COUNT 8


//=======================================================
//=======================================================
typedef enum
{
  BATTERY_NONE      = 0,
  BATTERY_INFINITE  = 1,
  BATTERY_DISCHAGED = 2,
  BATTERY_3MIN      = 3,
  BATTERY_10MIN     = 4,
  BATTERY_30MIN     = 5
} TBatteryEmulationType;


//======================================================
//======================================================
class TSimData
{
public:
  //---- gps ---

  bool GPSHasNewData;
  uint32_t gps_lastUpdate;

  int gps_fix;
	int gps_numSat;
  int gps_spoofing;

  //degrees, the latitude of the aircraft
	XPLMDataRef df_lattitude;
	double lattitude;

  //degrees, the longitude of the aircraft
	XPLMDataRef df_longitude;
	double longitude;

  //meters, the elevation above MSL of the aircraft
	XPLMDataRef df_elevation;
	double elevation;

  //meters/s, The velocity in local OGL coordinates
  XPLMDataRef df_local_vx;
  float local_vx;

  XPLMDataRef df_local_vy;
  float local_vy;

  XPLMDataRef df_local_vz;
  float local_vz;

  //meters, altitude Above Ground Level
	//XPLMDataRef df_agl;
	//float agl; 

  //meters/sec, the ground speed of the aircraft
	XPLMDataRef df_speed;
	float speed;

  //degrees, the true heading of the aircraft in degrees from the Z axis - OpenGL coordinates
  float course; //== yaw with GPS update rate
  //---- gps ---

  //degrees, the roll of the aircraft in degrees - OpenGL coordinates
	XPLMDataRef df_roll;
	float roll;

  //degrees, the pitch relative to the plane normal to the Y axis in degrees - OpenGL coordinates
	XPLMDataRef df_pitch;
	float pitch;

  //degrees, the true heading of the aircraft in degrees from the Z axis - OpenGL coordinates
	XPLMDataRef df_yaw;
	float yaw; // heading

	// accel
  //Gs, total g-forces on the plane as a multiple, along the plane, forward - negative
	XPLMDataRef df_accel_x;
	float accel_x;

  //Gs, total g-forces on the plane as a multiple, sideways, right - negative
	XPLMDataRef df_accel_y;
	float accel_y;

  //Gs, Total g-forces on the plane as a multiple, downward
	XPLMDataRef df_accel_z;
	float accel_z;

	// gyro
  //degrees, the roll of the aircraft in degrees - OpenGL coordinates
	XPLMDataRef df_gyro_x;
  float gyro_x;

  //deg/sec, the pitch rotation rates (relative to the flight)
	XPLMDataRef df_gyro_y;
	float gyro_y;

  //deg/sec	The yaw rotation rates (relative to the flight)
	XPLMDataRef df_gyro_z;
	float gyro_z;

	// baro
  //29.92+-....	This is the barometric pressure at the point the current flight is at.
	XPLMDataRef df_baro;
	float baro;

  bool simulatePitot;

  //meters/sec, the ground speed of the aircraft
  XPLMDataRef df_airspeed;
  float airspeed;

	//---- from inav --------  

  XPLMDataRef df_override_joystick;

	XPLMDataRef df_control_throttle;
	int16_t control_throttle;
	XPLMDataRef df_control_roll;
  int16_t control_roll;
	XPLMDataRef df_control_pitch;
  int16_t control_pitch;
	XPLMDataRef df_control_yaw;
  int16_t control_yaw;

  float estimated_attitude_roll;
  float estimated_attitude_pitch;
  float estimated_attitude_yaw;

  bool isAirplane;
  bool isArmed;
  bool isOSDDisabled;
  bool isOSDAnalogOSDNotFound;

  //-- state --
  bool muteBeeper;
  bool attitude_use_sensors;

  uint32_t lastUpdateMS;

  //battery state
  TBatteryEmulationType batEmulation;
  uint32_t battery_lastUpdate;
  float battery_chargeV;

	void init();

  void updateFromXPlane();
	void sendToXPlane();

  void updateFromINAV(const TMSPSimulatorFromINAV* data);
  void sendToINAV();

  void computeQuaternionFromRPY(float* quat, int16_t roll, int16_t pitch, int16_t yaw);
  void transformVectorEarthToBody(float* v, float* quat);
  void quaternionRotateVector(float* result, const float* vect, const float* quat);
  void quaternionConjugate(float* result, const float* q);
  void quaternionMultiply(float* result, const float* a, const float* b);

  void disconnect();

  void setBateryEmulation(TBatteryEmulationType s);
  void recalculateBattery();
};

extern TSimData g_simData;

