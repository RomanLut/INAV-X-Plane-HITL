#pragma once

#include "config.h"

#include "msp.h"

#define GPS_NO_FIX  0
#define GPS_FIX_2D  1
#define GPS_FIX_3D  2

#define DEBUG_U32_COUNT 8

//======================================================
//======================================================
class TSimData
{
public:
	// gps
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

  //meters, altitude Above Ground Level
	XPLMDataRef df_agl;
	float agl; 

  //meters/sec, the ground speed of the aircraft
	XPLMDataRef df_speed;
	float speed;

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

	// velocity
	//XPLMDataRef df_velocity_x;
	//float velocity_x;
	//XPLMDataRef df_velocity_y;
	//float velocity_y;
	//XPLMDataRef df_velocity_z;
	//float velocity_z;

	//---- output --------  SIM_INPUTS

	XPLMDataRef df_out_throttle;
	float out_throttle;
	XPLMDataRef df_out_roll;
	float out_roll;
	XPLMDataRef df_out_pitch;
	float out_pitch;
	XPLMDataRef df_out_yaw;
	float out_yaw;

  //-- state --
  bool emulateBattery;
  bool muteBeeper;

	void init();

  void updateFromXPlane();
	void sendToXPlane();

  void updateFromINAV(const TMSPSimulatorFromINAV* data);
  void sendToINAV();

  void disconnect();

};

extern TSimData g_simData;

