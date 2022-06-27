#include "simData.h"
#include "stats.h"

#include <math.h>

TSimData g_simData;

//==============================================================
//==============================================================
void TSimData::init()
{
  this->gps_fix = GPS_FIX_3D;
  this->gps_numSat = 12;
  this->gps_spoofing = 0;

  this->df_lattitude = XPLMFindDataRef("sim/flightmodel/position/latitude");
  this->df_longitude = XPLMFindDataRef("sim/flightmodel/position/longitude");
  this->df_elevation = XPLMFindDataRef("sim/flightmodel/position/elevation");
  this->df_agl = XPLMFindDataRef("sim/flightmodel/position/y_agl");

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

	//---- output ----
  this->emulateBattery = true;
  this->muteBeeper = true;
  this->attitude_use_sensors = false;

  this->df_control_throttle = XPLMFindDataRef("sim/cockpit2/engine/actuators/throttle_ratio_all");
  this->df_control_roll = XPLMFindDataRef("sim/joystick/yoke_roll_ratio");
  this->df_control_pitch = XPLMFindDataRef("sim/joystick/yoke_pitch_ratio");
  this->df_control_yaw = XPLMFindDataRef("sim/joystick/yoke_heading_ratio");
}

//==============================================================
//==============================================================
void TSimData::updateFromXPlane()
{
	this->lattitude = XPLMGetDatad(this->df_lattitude);
  this->longitude = XPLMGetDatad(this->df_longitude);
  this->elevation = XPLMGetDatad(this->df_elevation);
  this->agl = XPLMGetDataf(this->df_agl);

  this->speed = XPLMGetDataf(this->df_speed);

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

  g_stats.debug[data->debugIndex & 7] = data->debugValue;

  this->isAircraft = (data->debugIndex & 128) != 0;
}

//==============================================================
//==============================================================
void TSimData::sendToINAV()
{
  TMSPSimulatorToINAV data;

  data.version = 1;

  data.flags = 1 | (this->emulateBattery? 2 : 0) | (this->muteBeeper ? 4 : 0) | (this->attitude_use_sensors ? 8 : 0);

  data.fix = this->gps_fix;
  data.numSat = (uint8_t)this->gps_numSat;
  data.lat = (int32_t)round(this->lattitude * 10000000);
  data.lon = (int32_t)round(this->longitude * 10000000);
  data.alt = (int32_t)round(this->elevation * 100); //expected by inav: elevation in cm
  data.speed = (int16_t)round(this->speed * 100); //expected by inav: ground speed cm/sec
  data.course = (int16_t)round(this->yaw * 10);  // expected by inav: deg * 10 

  data.roll =  (int16_t)round( this->roll * 10);  //expected by inav: left wing down - negative roll, 1 degree = 10, values range: -1800...1800
  data.pitch = (int16_t)round(-this->pitch * 10);  //expected by inav: nose up - negative pitch, 1 degree = 10 , values range: -1800...1800
  data.yaw =   (int16_t)round( this->yaw * 10);  //expected by inav: rotate clockwise( top view) - positive yaw+, 1 degreee = 10 , values range: 0...3600

  g_stats.dbg_roll = data.roll;
  g_stats.dbg_pitch = data.pitch;
  g_stats.dbg_yaw = data.yaw;

  data.accel_x = (int16_t)round(-this->accel_x * 1000);  //expected by inav: forward - positive
  data.accel_y = (int16_t)round( this->accel_y * 1000);  //expected by inav: right - negative
  data.accel_z = (int16_t)round( this->accel_z * 1000);  //expected by inav: 1.0f in stable position (1G)

  g_stats.dbg_acc_x = data.accel_x / 1000.0f;
  g_stats.dbg_acc_y = data.accel_y / 1000.0f;
  g_stats.dbg_acc_z = data.accel_z / 1000.0f;

  data.gyro_x = (int16_t)round( this->gyro_x * 16);  //expected by inav: roll left wing down rotation -> negative
  data.gyro_y = (int16_t)round(-this->gyro_y * 16);  //expected by inav: pitch up rotation -> negative, 1 deerees per second
  data.gyro_z = (int16_t)round(-this->gyro_z * 16);  //expected by inav: yaw clockwise rotation (top view) ->negative

  g_stats.dbg_gyro_x = data.gyro_x / 16.0f;
  g_stats.dbg_gyro_y = data.gyro_y / 16.0f;
  g_stats.dbg_gyro_z = data.gyro_z / 16.0f;

  data.baro = (int32_t)round(this->baro * 3386.39f);

  g_msp.sendCommand(MSP_SIMULATOR, &data, sizeof(data));
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

