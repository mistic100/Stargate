#pragma once

#include <Servo.h>
#include <Stepper.h>
#include "random8.h"

#define NB_ANIMS 6

class Manager {

  private:
  enum Modes mode = ANIM;
  uint8_t animId = random8(0, NB_ANIMS);
  
  int ldrValue = 0;
  bool calibrated = false;
  
  uint8_t animStep = 0;
  uint8_t calibStep = 0;
  uint8_t dialStep = 0;
  
  int gatePosition = 0;
  int address[7];

  Stepper* gate;
  Servo* chevron;

  public:
  void init(Servo* _chevron, Stepper* _gate) {
    gate = _gate;
    chevron = _chevron;

    chevron->write(SERVO_CHEVRON_POS_UP);
    enableServoChevron();
    delay(SERVO_CHEVRON_DELAY);
    disableServoChevron();

    Serial.println("Ready");
  }

  /**
   * Change animation
   */
  void nextAnimation() {
    if (mode != ANIM) {
      setMode(ANIM);
      animId = 0;
    } else {
      animId++;
      if (animId >= NB_ANIMS) {
        animId = 0;
      }
    }

    if (mode == ANIM) {
      Serial.print("Start animation ");
      Serial.println(animId + 1);
      disengageAll();
      animStep = 0;
    }
  }

  /**
   * Dial a random address
   */
  void randomAddress() {
    if (!calibrated) {
      setMode(CAL);
      return;
    }
    
    Serial.print("Random address : ");
    for (uint8_t i = 0; i < 6; i++) {
      uint8_t a = random8(1, NB_SYMBOLS);
      Serial.print(a);
      Serial.print(",");
      address[i] = a;
    }
    Serial.println("0");
    address[6] = 0;
    
    dialStep = 0;
    
    setMode(DIAL);
  }

  /**
   * Main loop
   */
  void run() {
    switch (mode) {
      case ANIM:
        switch (animId) {
          case 0: runAnim1(); break;
          case 1: runAnim2(); break;
          case 2: runAnim3(); break;
          case 3: runAnim4(); break;
          case 4: runAnim5(); break;
          case 5: runAnim6(); break;
        }
        break;

      case CAL:
        switch (calibStep) {
          case 0: calibLDR(); break;
          case 1: calibPosition(); break;
        }
        break;

      case DIAL:
        dial();
        break;
    }
  }

  private:
  /**
   * Try changing current mode
   */
  void setMode(Modes newMode) {
    if (mode == CAL) {
      Serial.println("/!\\ Calibration in progress");
    } else {
      Serial.print("Change mode to : ");

      switch (newMode) {
        case ANIM:
          Serial.println("ANIM");
          break;
  
        case CAL:
          Serial.println("CAL");
          break;
  
        case DIAL:
          Serial.println("DIAL");
          break;
  
        case WAIT:
          Serial.println("WAIT");
          break;
      }
      
      mode = newMode;
      
      disengageAll();
      disableMotorGate();
      disableServoChevron();
    }
  }

  /**
   * Calibrate LDR sensibility
   */
  void calibLDR() {
    Serial.println("Calibrate LDR");

    enableMotorGate();
    digitalWrite(CALIBRATE_LED_PIN, HIGH);
    analogRead(CALIBRATE_LDR_PIN); // warm-up
    delay(1000);

    int ldrSum = 0;
    for (uint8_t i = 0; i < 10; i++) {
      int ldr = analogRead(CALIBRATE_LDR_PIN);
      Serial.print("LDR value : ");
      Serial.println(ldr);
      ldrSum+= ldr;
      gate->step(SYMBOL_STEP / 2);
    }

    int ldrAvg = ldrSum / 10;
    ldrValue = ldrAvg < 100 ? 100 : ldrAvg * 1.5;
    
    Serial.print("LDR Average : ");
    Serial.println(ldrAvg);
    Serial.print("LDR Limit : ");
    Serial.println(ldrValue);

    calibStep = 1;

    delay(1000);
    Serial.println("Calibrate position");
  }

  /**
   * Calibrate origin position
   */
  void calibPosition() {
    gate->step(1);
 
    int ldr = analogRead(CALIBRATE_LDR_PIN);
    Serial.print("LDR value : ");
    Serial.println(ldr);
    
    if (ldr > ldrValue) {
      Serial.println("Position calibrated");
      digitalWrite(CALIBRATE_LED_PIN, LOW);
      disableMotorGate();
      gatePosition = 0;
      mode = WAIT;
      calibrated = true;
    }
  }

  /**
   * Perform dialing
   */
  void dial() {
    // find shortest movement
    int stepsA = address[dialStep] - gatePosition;
    int stepsB = 39 - stepsA;
    int steps;
    int dir;
    if (abs(stepsA) < abs(stepsB)) {
      steps = abs(stepsA);
      dir = stepsA / steps;
    } else {
      steps = abs(stepsB);
      dir = - stepsB / steps;
    }

    if (steps == 0) {
      Serial.println("Chevron locked");

      enableServoChevron();
      delay(SERVO_CHEVRON_DELAY);
      chevron->write(SERVO_CHEVRON_POS_DOWN);
      digitalWrite(Chevron_Lock, HIGH);
      digitalWrite(Chevrons[dialStep], HIGH);
      
      delay(1000);

      if (dialStep < 6) {
        digitalWrite(Chevron_Lock, LOW);
      }
      chevron->write(SERVO_CHEVRON_POS_UP);
      delay(SERVO_CHEVRON_DELAY);
      disableServoChevron();

      dialStep++;
      
    } else {
      Serial.print(steps);
      Serial.print(dir < 0 ? " backward" : " forward");
      Serial.println(" steps remaining");
  
      enableMotorGate();
      gate->step(dir * SYMBOL_STEP);
      
      gatePosition += dir;
      if (gatePosition >= NB_SYMBOLS) {
        gatePosition-= NB_SYMBOLS;
      }
      else if (gatePosition < 0) {
        gatePosition+= NB_SYMBOLS;
      }
    }
    
    if (dialStep == 7) {
      Serial.println("Dialing done");
      disableMotorGate();
      engageAll();
      delay(5000);
      disengageAll();
      mode = WAIT;
    }
  }
  
  /**
   * Switch on all chevrons
   */
  void engageAll() {
    for (uint8_t i=0; i<9; i++) {
      digitalWrite(Ring_Chevrons[i], HIGH);
    }
  }
  
  /**
   * Switch off all chevrons
   */
  void disengageAll() {
    for (uint8_t i=0; i<9; i++) {
      digitalWrite(Ring_Chevrons[i], LOW);
    }
  }

  /**
   * Execute light animation 1
   */
  void runAnim1() {
    digitalWrite(Ring_Chevrons[animStep], HIGH);
    delay(20);
    if (animStep == 0){
      digitalWrite(Ring_Chevrons[8], LOW);
    } else { 
      digitalWrite(Ring_Chevrons[animStep -1], LOW);
    }
    delay(120);
    
    animStep++;
    if (animStep >= 9) {
      animStep = 0;
    }
  }

  /**
   * Execute light animation 2
   */
  void runAnim2() {
    if (animStep < 9){
      digitalWrite(Ring_Chevrons[animStep], HIGH);
    } else {
      digitalWrite(Ring_Chevrons[animStep - 9], LOW);
    }
    delay(120);
  
    animStep++;
    if (animStep >= 18) {
      animStep = 0;
    }
  }

  /**
   * Execute light animation 3
   */
  void runAnim3() {
    disengageAll();

    switch (animStep) {
      case 0: 
        digitalWrite(Ring_Chevrons[0], HIGH);
        break;
      case 1:
      case 2:
      case 16:
        digitalWrite(Ring_Chevrons[0], HIGH);
        digitalWrite(Ring_Chevrons[1], HIGH);
        digitalWrite(Ring_Chevrons[8], HIGH);
        break;
      case 3:
      case 15:
        digitalWrite(Ring_Chevrons[1], HIGH);
        digitalWrite(Ring_Chevrons[8], HIGH);
        break;
      case 4:
      case 14:
        digitalWrite(Ring_Chevrons[1], HIGH);
        digitalWrite(Ring_Chevrons[8], HIGH);
        digitalWrite(Ring_Chevrons[2], HIGH);
        digitalWrite(Ring_Chevrons[7], HIGH);
        break;
      case 5:
      case 13:
        digitalWrite(Ring_Chevrons[2], HIGH);
        digitalWrite(Ring_Chevrons[7], HIGH);
        break;
      case 6:
      case 12:
        digitalWrite(Ring_Chevrons[2], HIGH);
        digitalWrite(Ring_Chevrons[7], HIGH);
        digitalWrite(Ring_Chevrons[3], HIGH);
        digitalWrite(Ring_Chevrons[6], HIGH);
        break;
      case 7:
      case 11:
        digitalWrite(Ring_Chevrons[3], HIGH);
        digitalWrite(Ring_Chevrons[6], HIGH);
        break;
      case 8:
      case 10:
        digitalWrite(Ring_Chevrons[3], HIGH);
        digitalWrite(Ring_Chevrons[6], HIGH);
        digitalWrite(Ring_Chevrons[4], HIGH);
        digitalWrite(Ring_Chevrons[5], HIGH); 
        break;
      case 9:
        digitalWrite(Ring_Chevrons[4], HIGH);
        digitalWrite(Ring_Chevrons[5], HIGH); 
        break;
    }

    delay(100);
    
    animStep++;
    if (animStep >= 16) {
      animStep = 0;
    }
  }

  /**
   * Execute light animation 4
   */
  void runAnim4() {
    for (uint8_t i = animStep; i < animStep + 3; i++) {
      digitalWrite(Ring_Chevrons[i % 9], HIGH);
    }
    if (animStep == 0) {
      digitalWrite(Ring_Chevrons[8], LOW);
    } else {
      digitalWrite(Ring_Chevrons[animStep - 1], LOW);
    }

    delay(120);
  
    animStep++;
    if (animStep >= 9) {
      animStep = 0;
    }
  }

  /**
   * Execute light animation 5
   */
  void runAnim5() {
    disengageAll();
    int8_t tmp_Ring_2 = animStep - 3;
    int8_t tmp_Ring_3 = animStep + 3;
    if (tmp_Ring_2 < 0) {
      tmp_Ring_2+= 9;
    }
    if (tmp_Ring_3 >= 9) {
      tmp_Ring_3-= 9;
    }
    digitalWrite(Ring_Chevrons[animStep], HIGH);
    digitalWrite(Ring_Chevrons[tmp_Ring_2], HIGH);
    digitalWrite(Ring_Chevrons[tmp_Ring_3], HIGH);
    delay(120);
  
    animStep++;
    if (animStep >= 9) {
      animStep = 0;
    }
  }

  /**
   * Execute light animation 6
   */
  void runAnim6() {
    digitalWrite(Ring_Chevrons[random8(0, 8)], HIGH);
    digitalWrite(Ring_Chevrons[random8(0, 8)], LOW);
    delay(40);
  }

  void enableServoChevron() {
    digitalWrite(SERVO_CHEVRON_ENA_PIN, HIGH);
  }

  void disableServoChevron() {
    digitalWrite(SERVO_CHEVRON_ENA_PIN, LOW);
  }

  void enableMotorGate() {
    digitalWrite(MOTOR_SYMBOLS_ENA_PIN, LOW);
  }

  void disableMotorGate() {
    digitalWrite(MOTOR_SYMBOLS_ENA_PIN, HIGH);
  }
  
};
