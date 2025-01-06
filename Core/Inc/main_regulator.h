/*
 * main_regulator.h
 *
 *  Created on: Dec 29, 2024
 *      Author: dominik
 */

#ifndef INC_MAIN_REGULATOR_H_
#define INC_MAIN_REGULATOR_H_

#include <stdint.h>
#include <string.h>

typedef struct {
    double Kp;       // Wzmocnienie proporcjonalne
    double Ki;       // Wzmocnienie calkujace
    double Kd;       // Wzmocnienie rozniczkujace
    double Kc_pi;			 // Anty wind-up gain PI
    double Kc_pid;			 // Anty wind-up gain PID
    double prevError;  // Poprzedni blad
    double previntegral;  // Skumulowany calkowity blad
    double outputMin;  // Minimalna wartosc wyjsciowa
    double outputMax;  // Maksymalna wartosc wyjsciowa
    double dt;				 // Okres probkowania


    double prevHffrYr;  // Poprzedni Yr
    double prevHffrYffr;  // Poprzedni Yffr
    double a1;						// Wsp filtru PI
    double a2;						// Wsp filtru PI
    double b1;						// Wsp filtru PI
} PID_TypeDef;

void PID_Init(PID_TypeDef *pid, double Kp, double Ki, double Kd, double outputMin, double outputMax, double dt);
double PID_Compute(PID_TypeDef *pid, double setpoint, double measured_value);
double PI_Compute(PID_TypeDef *pid, double setpoint, double measured_value);
double saturation(double output, double outputMax, double outputMin);

#endif
