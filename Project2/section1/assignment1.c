// UCSD CSE237A - WI18
// Important! You WILL SUBMIT this file.
// Replace this with your implementation of Section 2
// For more details, please see the instructions in the class website.

#include "section1.h"
#include "shared_var.h"
#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>

//int buttonPress;
//int smallAudioSensor;
//int bigAudioSensor;

void program_init(SharedVariable* sv) {

  /*Initialize shared variables.*/
	sv->bProgramExit = 0;

                // You also need to initalize sensors here

  /* Define Input Interfaces*/
  pinMode (PIN_BUTTON, INPUT);
  pinMode (PIN_SMALL, INPUT);
  pinMode (PIN_BIG, INPUT);


  /*Create the SMD RGB led PWM*/
  softPwmCreate(PIN_SMD_RED, 0, 0xFF);
  softPwmCreate(PIN_SMD_GRN, 0, 0xFF);
  softPwmCreate(PIN_SMD_BLU, 0, 0xFF);

  /*Create the DIP RGB leds PWM*/
  softPwmCreate(PIN_DIP_RED, 0, 0xFF);
  softPwmCreate(PIN_DIP_GRN, 0, 0xFF);
  softPwmCreate(PIN_DIP_BLU, 0, 0xFF);

            /*Define Output Interfaces*/
  pinMode (PIN_ALED, OUTPUT); // define AUTOFLASH led as output interface
  
  pinMode (PIN_DIP_RED, OUTPUT); // define DIP_RED as output interface
  pinMode (PIN_DIP_GRN, OUTPUT); // define DIP_GRN as output interface
  pinMode (PIN_DIP_BLU, OUTPUT); // define DIP_BLU as output interface

  pinMode (PIN_SMD_RED, OUTPUT); // define SMD_RED as output interface
  pinMode (PIN_SMD_GRN, OUTPUT); // define SMD_GRN as output interface
  pinMode (PIN_SMD_BLU, OUTPUT); // define SMD_BLU as output interface

}

static void inline readSensors(SharedVariable* sv) {
  sv->buttonPress = digitalRead(PIN_BUTTON);
  sv->smallAudioSensor = digitalRead(PIN_SMALL);
  sv->bigAudioSensor = digitalRead(PIN_BIG);
}

static void inline SMD_LED_Set(int red, int green, int blue) {
  softPwmWrite(PIN_SMD_RED, red); //
  softPwmWrite(PIN_SMD_GRN, green);//
  softPwmWrite(PIN_SMD_BLU, blue);//
}

static void inline DIP_LED_Set(int red, int green, int blue) {
  softPwmWrite(PIN_DIP_RED, red); //DIP RGB LED is RED if small audio sensor gives 1
  softPwmWrite(PIN_DIP_GRN, green); //turn off if previously turned on
  softPwmWrite(PIN_DIP_BLU, blue); //turn off if previously turned on

}

static void inline SMDCyan(SharedVariable* sv) {
  //SMD RGB LED is CYAN (RGB = 0x00, 0xff, 0xff)
  SMD_LED_Set(0x00, 0xff, 0xff);
//  softPwmWrite(PIN_SMD_RED, 0x00); //
//  softPwmWrite(PIN_SMD_GRN, 0xff);//
//  softPwmWrite(PIN_SMD_BLU, 0xff);//
}

static void inline SMDPurple(SharedVariable* sv) {
  //SMD RGB LED is PURPLE (RGB = 0xee, 0x00, 0xc8)
  softPwmWrite(PIN_SMD_RED, 0xee); //
  softPwmWrite(PIN_SMD_GRN, 0x00);//
  softPwmWrite(PIN_SMD_BLU, 0xc8);//
}
static void inline SMDYellow(SharedVariable* sv) {
  //SMD RGB LED is YELLOW (RGB = 0x80, 0xff, 0x00)
  softPwmWrite(PIN_SMD_RED, 0x80); //
  softPwmWrite(PIN_SMD_GRN, 0xff);//
  softPwmWrite(PIN_SMD_BLU, 0x00);//
}
static void inline SMDRed(SharedVariable* sv) {
  //SMD RGB LED is RED (RGB = 0xff, 0x00, 0x00)
  softPwmWrite(PIN_SMD_RED, 0xff); //
  softPwmWrite(PIN_SMD_GRN, 0x00);//
  softPwmWrite(PIN_SMD_BLU, 0x00);//
}
static void inline DIPRed(SharedVariable* sv) {
  softPwmWrite(PIN_DIP_BLU, 0x00); //turn off if previously turned on
  softPwmWrite(PIN_DIP_RED, 0xff); //DIP RGB LED is RED if small audio sensor gives 1
}
static void inline DIPBlue(SharedVariable* sv) {
  softPwmWrite(PIN_DIP_RED, 0x00); //turn off if previously turned on
  softPwmWrite(PIN_DIP_BLU, 0xff); //DIP RGB LED is BLUE if small audio sensor gives 0

}

static void inline smallSoundOff(SharedVariable* sv) {
  //DIP RGB LED is BLUE if small audio sensor gives 0
  DIPBlue(sv);
  //0 1 case => Yellow
  if (sv->bigAudioSensor == HIGH){
    SMDYellow(sv);
  }
    //0 0 case => Red
  else {//bigAudioSensor == LOW
    SMDRed(sv);
  }
}

static void inline smallSoundOn(SharedVariable* sv) {
  //DIP RGB LED is RED if small audio sensor gives 1
  DIPRed(sv);
        //1 1 case => Cyan
  if (sv->bigAudioSensor == HIGH){
    SMDCyan(sv);
  }
      //1 0 case => Purple
  else {//bigAudioSensor == LOW
    SMDPurple(sv);
  }
}

static void inline runningState(SharedVariable* sv){
  digitalWrite(PIN_ALED, HIGH);  //AUTO-FLASH led on

  // 1 - case
  if (sv->smallAudioSensor == HIGH){
   smallSoundOn(sv);
  }
    // 0 - case
  else{//smallAudioSensor == LOW
    smallSoundOff(sv);
  }
}

static void inline pauseState(SharedVariable* sv) {
  // buttonPress == LOW
  digitalWrite(PIN_ALED, LOW);  //AUTO-FLASH led off

  //turn off DIP leds
  DIP_LED_Set(0x00,0x00,0x00);
//
//  softPwmWrite(PIN_DIP_RED, 0x00); //turn off if previously turned on
//  softPwmWrite(PIN_DIP_GRN, 0x00); //turn off for good measure
//  softPwmWrite(PIN_DIP_BLU, 0x00); //turn off if previously turned on

  //turn off SMD leds
  SMD_LED_Set(0x00, 0x00, 0x00);
//
//  softPwmWrite(PIN_SMD_RED, 0x00); //turn off if previously turned on
//  softPwmWrite(PIN_SMD_GRN, 0x00); //turn off if previously turned on
//  softPwmWrite(PIN_SMD_BLU, 0x00); //turn off if previously turned on
}

void program_body(SharedVariable* sv) {
    // Implement your sensor handling procedure.
    // Keep also this in mind:

    // - Make this fast and efficient.
    //   In Section 2, it's a part of scheduled tasks.
    //   For example, if it is slow, this will degradade energy efficiency.

    // - So, don't make any loop (e.g., don't use "for" & "while")
    //   This would hurt the performance of the task.

    // - Don't make any delay using delay(), sleep(), etc

  readSensors(sv);

                    /*RUNNING State*/
  if (sv->buttonPress == HIGH){
    runningState(sv);
  }
                  /*PAUSE state*/
  else{
    pauseState(sv);
  }
}

void program_exit(SharedVariable* sv) {
    // Clean up everything if needed.
    // This is called when the program finishes.
  softPwmStop(PIN_SMD_RED);
  softPwmStop(PIN_SMD_GRN);
  softPwmStop(PIN_SMD_BLU);

  softPwmStop(PIN_DIP_RED);
  softPwmStop(PIN_DIP_GRN);
  softPwmStop(PIN_DIP_BLU);
}

