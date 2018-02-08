// UCSD CSE237A - WI18
// Important! You WILL SUBMIT this file.
// Replace this with your implementation of Section 2
// For more details, please see the instructions in the class website.

#include "section1.h"
#include "shared_var.h"
#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>

void program_init(SharedVariable* sv) {

  /*Initialize shared variables.*/
  sv->bProgramExit = 0;
  sv->running = 1;
  sv->smallOn = 0;
  sv->bigOn = 0;

                // You also need to initalize sensors here
  printf("Initializing INPUTS.\n");

  /* Define Input Interfaces*/
  pinMode (PIN_BUTTON, INPUT);
  pinMode (PIN_SMALL, INPUT);
  pinMode (PIN_BIG, INPUT);

  printf("Initializing OUTPUTS.\n");

  /*Define Output Interfaces*/
  pinMode (PIN_ALED, OUTPUT); // define AUTOFLASH led as output interface

  printf("Initializing SMD.\n");

  /*Create the SMD RGB led PWM*/
  softPwmCreate(PIN_SMD_RED, 0, 0xFF);
  softPwmCreate(PIN_SMD_GRN, 0, 0xFF);
  softPwmCreate(PIN_SMD_BLU, 0, 0xFF);

  printf("Initializing DIP.\n");

  /*Create the DIP RGB leds PWM*/
  softPwmCreate(PIN_DIP_RED, 0, 0xFF);
  softPwmCreate(PIN_DIP_GRN, 0, 0xFF);
  softPwmCreate(PIN_DIP_BLU, 0, 0xFF);

}



static void inline checkButton(SharedVariable* sv) {

  if(digitalRead(PIN_BUTTON) == LOW){
    while(digitalRead(PIN_BUTTON) == LOW)
      ;// eat up remainder button press
    sv->running = !sv->running;
  }

}


static void inline checkMicrophones(SharedVariable* sv) {
  sv->smallOn = digitalRead(PIN_SMALL);
  sv->bigOn = digitalRead(PIN_BIG);
}
static void inline SMD_LED_Set(unsigned char red, unsigned char green, unsigned char blue) {
  softPwmWrite(PIN_SMD_RED, red); //
  softPwmWrite(PIN_SMD_GRN, green);//
  softPwmWrite(PIN_SMD_BLU, blue);//
//  delay(1);
}

static void inline DIP_LED_Set(unsigned char red, unsigned char green, unsigned char blue) {
  softPwmWrite(PIN_DIP_RED, red); //DIP RGB LED is RED if small audio sensor gives 1
  softPwmWrite(PIN_DIP_GRN, green); //turn off if previously turned on
  softPwmWrite(PIN_DIP_BLU, blue); //turn off if previously turned on
//  delay(1);
}

static void inline SMDCyan(SharedVariable* sv) {
  //SMD RGB LED is CYAN (RGB = 0x00, 0xff, 0xff)
  SMD_LED_Set(0x00, 0xff, 0xff);
}

static void inline SMDPurple(SharedVariable* sv) {
  //SMD RGB LED is PURPLE (RGB = 0xee, 0x00, 0xc8)
    SMD_LED_Set(0xee, 0x00, 0xc8);

}
static void inline SMDYellow(SharedVariable* sv) {
  //SMD RGB LED is YELLOW (RGB = 0x80, 0xff, 0x00)
    SMD_LED_Set(0x80, 0xff, 0x00);
}
static void inline SMDRed(SharedVariable* sv) {
  //SMD RGB LED is RED (RGB = 0xff, 0x00, 0x00)
    SMD_LED_Set(0xff, 0x00, 0x00);
}
static void inline DIPRed(SharedVariable* sv) {
    //DIP RGB LED is RED if small audio sensor gives 1
  DIP_LED_Set(0xff, 0x00, 0x00);
}
static void inline DIPBlue(SharedVariable* sv) {
    //DIP RGB LED is BLUE if small audio sensor gives 0
    DIP_LED_Set(0x00, 0x00, 0xff);
}

static void inline runningState(SharedVariable* sv){
//  printf("ALED on.\n");
  digitalWrite(PIN_ALED, HIGH);  //AUTO-FLASH led on

  // 1 - case
  if (sv->smallOn == HIGH && sv->bigOn == HIGH){
    DIPRed(sv);
    SMDCyan(sv);
  }else if (sv->smallOn == LOW && sv->bigOn == HIGH){
    DIPBlue(sv);
    SMDYellow(sv);
  }else if (sv->smallOn == HIGH && sv->bigOn == LOW){
    DIPRed(sv);
    SMDPurple(sv);
  } else{//low low
    DIPBlue(sv);
    SMDRed(sv);
  }

}

static void inline pauseState(SharedVariable* sv) {
//  printf("ALED off.\n");
  digitalWrite(PIN_ALED, LOW);  //AUTO-FLASH led off
  //unset microphone sensor values.
  sv->smallOn = 0;
  sv->bigOn = 0;

  //turn off DIP leds
  DIP_LED_Set(0x00,0x00,0x00);

  //turn off SMD leds
  SMD_LED_Set(0x00, 0x00, 0x00);
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

  checkButton(sv);

                    /*RUNNING State*/
  if (sv->running == 1){
//    printf("RUN state.\n");
    checkMicrophones(sv);
    runningState(sv);
  }
                  /*PAUSE state*/
  else{
//    printf("PAUSE state.\n");
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

