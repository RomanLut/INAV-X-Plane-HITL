rotSpeed = find_dataref("sim/flightmodel2/engines/engine_rotation_speed_rad_sec[0]")
motorPower = find_dataref("sim/flightmodel/engine/ENGN_power[0]")
throttle = find_dataref("sim/flightmodel2/engines/throttle_used_ratio[0]")
motorSound = create_dataref( "nk_fpv_surfwing/motor_sound", "number")
motorSoundAcc = create_dataref( "nk_fpv_surfwing/motor_sound_acc", "number")
motorSoundAcc2 = create_dataref( "nk_fpv_surfwing/motor_sound_acc2", "number")

function after_physics()

s = ( throttle - 0.042 ) / ( 0.849 - 0.042 )
s = math.max( s, 0 )

--s = ( s - 21 ) / ( 780 - 21 )
--s = math.max( s, 0 )

if s > 0 then
  motorSoundAcc = motorSoundAcc * 0.9 + s * 0.1
else
  motorSoundAcc = motorSoundAcc * 0.7 + s * 0.3
end

if motorSoundAcc > 0.0001 then
  motorSoundAcc2 = motorSoundAcc2 + 0.01
else
  motorSoundAcc2 = motorSoundAcc2 - 0.1
end

motorSoundAcc2 = math.max( motorSoundAcc2, 0 )
motorSoundAcc2 = math.min( motorSoundAcc2, 1 )

motorSound = ( math.sqrt( motorSoundAcc ) * 0.95 + motorSoundAcc2 * motorSoundAcc2 * motorSoundAcc2 * motorSoundAcc2 * 0.05) * 2000

end  
 