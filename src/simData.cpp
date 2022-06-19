#include "simData.h"

#include <math.h>

TSimData g_simData;

//==============================================================
//==============================================================
void TSimData::init()
{
  gps_fix = 2;
  gps_numSat = 12;
  gps_spoofing = 0;

	df_lattitude = XPLMFindDataRef("sim/flightmodel/position/latitude");
	df_longitude = XPLMFindDataRef("sim/flightmodel/position/longitude");
	df_elevation = XPLMFindDataRef("sim/flightmodel/position/elevation");
	df_agl = XPLMFindDataRef("sim/flightmodel/position/y_agl");

	df_speed = XPLMFindDataRef("sim/flightmodel/position/groundspeed");

	df_roll = XPLMFindDataRef("sim/flightmodel/position/phi");
	df_pitch = XPLMFindDataRef("sim/flightmodel/position/theta");
	df_yaw = XPLMFindDataRef("sim/flightmodel/position/psi");

	// Accelerometer
	df_accel_x = XPLMFindDataRef("sim/flightmodel/forces/g_axil");
	df_accel_y = XPLMFindDataRef("sim/flightmodel/forces/g_side");
	df_accel_z = XPLMFindDataRef("sim/flightmodel/forces/g_nrml");

	// Gyro
	df_gyro_x = XPLMFindDataRef("sim/flightmodel/position/P");
	df_gyro_y = XPLMFindDataRef("sim/flightmodel/position/Q");
	df_gyro_z = XPLMFindDataRef("sim/flightmodel/position/R");

	// Barometer
	df_baro = XPLMFindDataRef("sim/weather/barometer_current_inhg");

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
	df_out_throttle = XPLMFindDataRef("sim/cockpit2/engine/actuators/throttle_ratio_all");
	df_out_roll = XPLMFindDataRef("sim/joystick/yoke_roll_ratio");
	df_out_pitch = XPLMFindDataRef("sim/joystick/yoke_pitch_ratio");
	df_out_yaw = XPLMFindDataRef("sim/joystick/yoke_heading_ratio");
}

//==============================================================
//==============================================================
void TSimData::updateFromXPlane()
{
	lattitude = XPLMGetDatad(df_lattitude);
	longitude = XPLMGetDatad(df_longitude);
	elevation = XPLMGetDatad(df_elevation);
	agl = XPLMGetDataf(df_agl);

	speed = XPLMGetDataf(df_speed);

	roll = XPLMGetDataf(df_roll);
	pitch = XPLMGetDataf(df_pitch);
	yaw = XPLMGetDataf(df_yaw);

	accel_x = XPLMGetDataf(df_accel_x);
	accel_y = XPLMGetDataf(df_accel_y);
	accel_z = XPLMGetDataf(df_accel_z);

	gyro_x = XPLMGetDataf(df_gyro_x);
	gyro_y = XPLMGetDataf(df_gyro_y);
	gyro_z = XPLMGetDataf(df_gyro_z);

	baro = XPLMGetDataf(df_baro);
}

//==============================================================
//==============================================================
void TSimData::sendToXPlane()
{
	XPLMSetDataf(df_out_throttle, out_throttle / 1000);
	XPLMSetDataf(df_out_roll, out_roll / 500 - 1);
	XPLMSetDataf(df_out_pitch, (out_pitch / 500 - 1)*-1);
	XPLMSetDataf(df_out_yaw, out_yaw*10*0);
}

//==============================================================
//==============================================================
void TSimData::updateFromINAV(const TMSPSimulatorFromINAV* data)
{
  this->out_throttle = data->throttle;
  this->out_roll = data->roll;
  this->out_pitch = data->pitch;
  this->out_yaw = data->yaw;
}

//==============================================================
//==============================================================
void TSimData::sendToINAV()
{
  TMSPSimulatorToINAV data;

  data.flags = 1; 

  data.fix = this->gps_fix;
  data.numSat = (uint8_t)this->gps_numSat;
  data.lat = (int32_t)(this->lattitude * 10000000 + 0.5);
  data.lon = (int32_t)(this->longitude * 10000000  + 0.5);
  data.alt = (int32_t)(this->elevation * 100 + 0.5f);
  data.speed = (int32_t)(this->speed * 100 + 0.5f);
  data.course = (int32_t)(this->yaw + 0.5f);

  data.roll = (int16_t)((this->roll + 180) * 10 + 0.5f);
  data.pitch = (int16_t)((this->pitch*-1 + 180) * 10 + 0.5f);
  data.yaw = (int16_t)(this->yaw * 10 + 0.5f);

  data.accel_x = (int16_t)((this->accel_x * -320) * 100 + 0.5f);
  data.accel_y = (int16_t)((this->accel_y * -320) * 100 + 0.5f);
  data.accel_z = (int16_t)((this->accel_z * 320) * 100 + 0.5f);

  data.gyro_x = (int16_t)((this->gyro_x * 320) * 100 + 0.5f);
  data.gyro_y = (int16_t)((this->gyro_y * -320) * 100 + 0.5f);
  data.gyro_z = (int16_t)((this->gyro_z * -320) * 100 + 0.5f);

  data.baro = (int32_t)(this->baro * 3386.39f + 0.5f);

  g_msp.sendCommand(MSP_SIMULATOR, &data, sizeof(data));
}

