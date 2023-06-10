#include <Wire.h>
#include <TinyMPU6050.h>

TwoWire Wire(1);
MPU6050 mpu(Wire);

float prev_yaw;
float smoothing_factor = 0.8;

long mapf(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void SetupMPU(){
  Serial.println("INITIALIZING MPU");
  mpu.Initialize();
  #ifdef CALIBRATE_MPU
  Serial.println(F("CALIBRATING MPU, DONOT MOVE"));
  delay(500);
  mpu.Calibrate();
  Serial.println("Calibration complete!");
  #endif
  Serial.println("MPU Offsets:");
  Serial.print("GyroX Offset = ");
  Serial.println(mpu.GetGyroXOffset());
  Serial.print("GyroY Offset = ");
  Serial.println(mpu.GetGyroYOffset());
  Serial.print("GyroZ Offset = ");
  Serial.println(mpu.GetGyroZOffset());
  Serial.println("Calibration complete!");
  Time = micros(); 
}

void ReadFromMPU() {
  mpu.Execute();
  
  angle_roll = mpu.GetAngY();
  angle_pitch = mpu.GetAngX();
  angle_yaw = mpu.GetAngZ();

  delta_yaw = (angle_yaw - prev_yaw) * 0.8;
  prev_yaw = angle_yaw;

  if(angle_roll < -100 || angle_pitch < -100){crashed = true;}
  if(crashed)return;

  roll_mapped = map(angle_roll, -90,90,-40,40);
  pitch_mapped = map(angle_pitch, -90,90,-40,40);
  yaw_mapped = mapf(delta_yaw, -2.0,2.0,-10.0,10.0);

  // Roll PID
  roll_error = angle_roll - input_ROLL;
  roll_integral += roll_error;
  roll_derivative = roll_error - roll_previous_error;
  roll_previous_error = roll_error;
  float roll_PID = roll_Kp * roll_error + roll_Ki * roll_integral + roll_Kd * roll_derivative;
  
  // Pitch PID
  pitch_error = angle_pitch - input_PITCH;
  pitch_integral += pitch_error;
  pitch_derivative = pitch_error - pitch_previous_error;
  pitch_previous_error = pitch_error;
  float pitch_PID = pitch_Kp * pitch_error + pitch_Ki * pitch_integral + pitch_Kd * pitch_derivative;
  
  // Yaw PID
  yaw_error = angle_yaw - input_YAW;
  yaw_integral += yaw_error;
  yaw_derivative = yaw_error - yaw_previous_error;
  yaw_previous_error = yaw_error;
  float yaw_PID = yaw_Kp * yaw_error + yaw_Ki * yaw_integral + yaw_Kd * yaw_derivative;
  
//  ESCout_1 = input_THROTTLE + roll_mapped - pitch_mapped + yaw_mapped;
//  ESCout_2 = input_THROTTLE - roll_mapped + pitch_mapped + yaw_mapped;
//  ESCout_3 = input_THROTTLE - roll_mapped - pitch_mapped - yaw_mapped;
//  ESCout_4 = input_THROTTLE + roll_mapped + pitch_mapped - yaw_mapped;

  ESCout_1 = input_THROTTLE + roll_PID - pitch_PID + yaw_PID;
  ESCout_2 = input_THROTTLE - roll_PID + pitch_PID + yaw_PID;
  ESCout_3 = input_THROTTLE - roll_PID - pitch_PID - yaw_PID;
  ESCout_4 = input_THROTTLE + roll_PID + pitch_PID - yaw_PID;


  minThrottle = input_THROTTLE;
  
  if(ESCout_1>maxThrottle) ESCout_1=maxThrottle;
  else if(ESCout_1<minThrottle) ESCout_1=minThrottle;
  
  if(ESCout_2>maxThrottle) ESCout_2=maxThrottle;
  else if(ESCout_2<minThrottle) ESCout_2=minThrottle;
  
  if(ESCout_3>maxThrottle) ESCout_3=maxThrottle;
  else if(ESCout_3<minThrottle) ESCout_3=minThrottle;
  
  if(ESCout_4>maxThrottle) ESCout_4=maxThrottle;
  else if(ESCout_4<minThrottle) ESCout_4=minThrottle;
}
