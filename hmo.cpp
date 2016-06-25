#include <Arduino.h>

// pin definitions /////////////////////////////////////////////////

#define PIN_LIGHT       A0 //measures ambiant light

// The following two pins are shared, assuming stepper and solenoid
// won't be used at the same time. So, make sure when using stepper
// motor voltage supply is off
#define PIN_STEP        A1 //stepper step
#define PIN_SOL_EN      A1 //solenoid enable

#define PIN_STEP_SLP    A2 //stepper sleep, active low
#define PIN_VREG_IN     A3 //measures voltage of VREG output
#define PIN_STEP_STOP   A4 //stepper stopper input
#define PIN_BL_CS       2  //backlight(APA102C) chip select
#define PIN_SHD_DRV     3  //shade driver
#define PIN_MT_DIR      4  //shade motor direction 
#define PIN_MT_STOP     5  //shade motor stopper input
#define PIN_PWR_SEL     6  //power select between shade and motor
#define PIN_VREG_EN     7  //VREG enable
#define PIN_SRV1        8  //servo 1 (mirror servo) pwm
#define PIN_SRV2        9  //servo 2 (eyepiece servo) pwm
#define PIN_POT_CS      10 //VREG digital pot chip select

// The following pins are multiplexed among pot SPI, backlight SPI and stepper.
// When STEPPER_SLP is off, i.e. when stepper is active, make sure both
// BACKLIGHT_CS and POT_CS are deselected.
#define PIN_MOSI        11 //atmega328 SPI master output
#define PIN_STEP_DIR    11 //stepper direction
#define PIN_BL_DIN      11 //backlight data in
#define PIN_POT_DIN     11 //VREG digitial pot data in
#define PIN_SOL_DIR     11 //solenoid direction
#define PIN_MISO        12 //atmega328 SPI master input
#define PIN_BL_DOUT     12 //backlight data out
#define PIN_POT_DOUT    12 //VREG digital pot data out
#define PIN_SCLK        13 //atmega328 SPI clock output
#define PIN_POT_CLK     13 //VREG digital pot clock in
#define PIN_BL_CLK      13 //backlight clock input
////////////////////////////////////////////////////////////

#define DECLARE_TIMEOUT(_p) static unsigned long _##_p##_start, _##_p##_tick
#define RESET_TIMEOUT(_p) _##_p##_start=millis()

// cheap timeout detection, when wrap around, the actual timeout may double
#define IS_TIMEOUT(_p,_timeout) \
    (_##_p##_tick=millis(),\
     (_##_p##_tick>=_##_p##_start&&(_##_p##_tick-_##_p##_start)>=_timeout) || \
        (_##_p##_tick<_##_p##_start&&_##_p##_tick>=_timeout))

// accurate timeout detection
#define IS_TIMEOUT2(_p,_timeout) \
    (_##_p##_tick=millis(),\
     (_##_p##_tick>=_##_p##_start&&(_##_p##_tick-_##_p##_start)>=_timeout) || \
        (_##_p##_tick<_##_p##_start&&((0xffffffff-_##_p##_start)+_##_p##_tick)>=_timeout))

#define UPDATE_TIMEOUT(_p) _##_p##_start = _##_p##_tick

#include "shell.h"
#include "sv.h"
#include "stepper.h"
#include "pot.h"
// #include "shade.h"
// #include "mt.h"

void setup() {
    pinMode(PIN_LIGHT,INPUT);

    digitalWrite(PIN_STEP_SLP,LOW);
    pinMode(PIN_STEP_SLP,OUTPUT);

    digitalWrite(PIN_VREG_EN,LOW);
    pinMode(PIN_VREG_EN,OUTPUT);

    pinMode(PIN_MT_DIR,OUTPUT);

    pinMode(PIN_VREG_IN,INPUT);

    digitalWrite(PIN_PWR_SEL,LOW);
    pinMode(PIN_PWR_SEL,OUTPUT);

    pinMode(PIN_SHD_DRV,INPUT);
    pinMode(PIN_MT_STOP,INPUT);
    pinMode(PIN_STEP_STOP,INPUT);

    digitalWrite(PIN_POT_CS,LOW);
    pinMode(PIN_POT_CS,OUTPUT);
    
    digitalWrite(PIN_BL_CS,LOW);
    pinMode(PIN_BL_CS,OUTPUT);

    setupShell();
    setupPot();
}

void loop() {
    loopShell();
}

