/*
 * main_regulator.c
 *
 *  Created on: Dec 29, 2024
 *      Author: domink
 */


#include "main_regulator.h"
#include <math.h>


void PID_Init(PID_TypeDef *pid, double Kp, double Ki, double Kd, double outputMin, double outputMax, double dt) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->Kc_pi = Ki;
    pid->Kc_pid = sqrt(Ki/Kd);
    pid->prevError = 0.0;
    pid->previntegral = 0.0;
    pid->outputMin = outputMin;
    pid->outputMax = outputMax;
    pid->dt = dt;

    pid->prevHffrYr = 0.0;
    pid->prevHffrYffr = 0.0;
    pid->a1 = dt * Ki / (dt * Ki + 2 * Kp);
    pid->a2 = dt * Ki / (dt * Ki + 2 * Kp);
    pid->b1 = 2 * Kp - (dt * Ki) / (dt * Ki + 2 * Kp);
}


/* Funkcja obliczająca wyjście regulatora PID */
double PID_Compute(PID_TypeDef *pid, double setpoint, double measured_value) {
  double error = setpoint - measured_value;

  double integral = pid->previntegral + (error + pid->prevError);  // Tustin
  pid->previntegral = integral;

  double derivative = (error - pid->prevError) / pid->dt;
  // U
  double output = pid->Kp * error + pid->Ki * integral * (pid->dt/2) + pid->Kd * derivative;

	// Ograniczenie wyjscia (saturacia)
  double output_sat = saturation(output, pid->outputMin, pid->outputMax);

	// Korekcja anty-windup
	double anti_windup = pid->Kc_pid * (output_sat - output);
	pid->previntegral += anti_windup;

  pid->prevError = error;

  return output;
}


/* Funkcja obliczająca wyjście regulatora PI */
double PI_Compute(PID_TypeDef *pid, double setpoint, double measured_value) {
	// Hffr = 1/(kp/ki)s+1  filtr w torze sygnalu referencyjnego
	float Yffr;
	if(pid->Kp/pid->Ki > 0 && 0){  // jezeli jest stabilny
	  Yffr = pid->a1 * setpoint + pid->a2 * pid->prevHffrYr + pid->b1 * pid->prevHffrYffr;
	  pid->prevHffrYr = setpoint;
	  pid->prevHffrYffr = Yffr;
	}
	else{
		Yffr = setpoint;
	}

	// PI regulator
  double error = Yffr - measured_value;
  // Ki
  double integral = pid->previntegral + (error + pid->prevError) * (pid->dt/2);  // Tustin
  pid->previntegral = integral;
  // U
  double output = pid->Kp * error + pid->Ki * integral;

	// Ograniczenie wyjscia (saturacia)
  double output_sat = saturation(output, pid->outputMin, pid->outputMax);

	// Korekcja anty-windup
	double anti_windup = pid->Kc_pi * (output_sat - output);
	pid->previntegral += anti_windup;

  pid->prevError = error;

  return output;
}


double saturation(double output, double outputMin, double outputMax){
  double output_sat;
	if (output > outputMax) {
		output_sat = outputMax;
	} else if (output < outputMin) {
		output_sat = outputMin;
	}else{
		output_sat = output;
	}
	return output_sat;
}
