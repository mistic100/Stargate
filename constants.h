#pragma once

#define SERVO_CHEVRON_PIN 13
#define SERVO_CHEVRON_ENA_PIN 11
#define SERVO_CHEVRON_POS_UP 105
#define SERVO_CHEVRON_POS_DOWN 75
#define SERVO_CHEVRON_DELAY 600

#define MOTOR_SYMBOLS_PIN1 A3
#define MOTOR_SYMBOLS_PIN2 A4
#define MOTOR_SYMBOLS_ENA_PIN A5
#define MOTOR_SYMBOLS_STEPS 200 * 4
#define MOTOR_SYMBOLS_RPM 60

// 78 teeth on symbols gear
// 12 teeth on motor gear
#define NB_SYMBOLS 39
#define SYMBOL_STEP 123 //78 / 12 * MOTOR_SYMBOLS_STEPS / NB_SYMBOLS

#define CONTROLS_PIN A1
#define CONTROLS_LONG_PRESS 500

#define CALIBRATE_LED_PIN A0
#define CALIBRATE_LDR_PIN A2

// Populate Chevrons[] array with output pins according to each chevron in dialling order.
int Chevrons[] = {9,8,7,2,5,4,6};

int Chevron_Lock = Chevrons[6];

//Populate Ring_Chevrons[] array with output pins according to each chevron in clockwise (or anticlockwise) order.
int Ring_Chevrons[] = {6,9,8,7,10,3,2,5,4};
