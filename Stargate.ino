#include <Servo.h>
#include <Stepper.h>
#include <CmdParser.hpp>

#include "constants.h"
#include "modes.h"
#include "manager.h"

Servo chevron;
Stepper gate(MOTOR_SYMBOLS_STEPS, MOTOR_SYMBOLS_PIN1, MOTOR_SYMBOLS_PIN2);
CmdParser cmdParser;
Manager manager;

void setup() {
  Serial.begin(9600);
  Serial.println("Stargate");
  
  pinMode(MOTOR_SYMBOLS_PIN1, OUTPUT);
  pinMode(MOTOR_SYMBOLS_PIN2, OUTPUT);
  pinMode(MOTOR_SYMBOLS_ENA_PIN, OUTPUT);
  pinMode(SERVO_CHEVRON_PIN, OUTPUT);
  pinMode(SERVO_CHEVRON_ENA_PIN, OUTPUT);
  pinMode(CALIBRATE_LED_PIN, OUTPUT);
  pinMode(CALIBRATE_LDR_PIN, INPUT);
  pinMode(CONTROLS_PIN, INPUT);
  for (int i = 0; i < 9; i ++){
    pinMode(Ring_Chevrons[i], OUTPUT);
  }
  
  digitalWrite(SERVO_CHEVRON_ENA_PIN, LOW); // disable servo
  digitalWrite(MOTOR_SYMBOLS_ENA_PIN, HIGH); // disable motor
  
  chevron.attach(SERVO_CHEVRON_PIN);
  gate.setSpeed(MOTOR_SYMBOLS_RPM);

  manager.init(&chevron, &gate);
}

void loop() {
  while (Serial.available() > 0) {
    CmdBuffer<32> myBuffer;

    if (myBuffer.readFromSerial(&Serial, 2000) && cmdParser.parseCmd(&myBuffer) != CMDPARSER_ERROR) {
      if (cmdParser.equalCommand_P(PSTR("ROT"))) {
        int steps = atoi(cmdParser.getCmdParam(1));
        Serial.print(steps);
        
        digitalWrite(MOTOR_SYMBOLS_ENA_PIN, LOW);
        gate.step(steps);
        digitalWrite(MOTOR_SYMBOLS_ENA_PIN, HIGH);
      }
    }
  }

  int control = analogRead(CONTROLS_PIN);
  if (control > CONTROLS_THRES_1) {
    manager.randomAddress();
  }
  else if (control > CONTROLS_THRES_2) {
    manager.nextAnimation();
  }
  
  manager.run();
}
