#include <Arduino.h>

// pin definitions /////////////////////////////////////////////////

#define PIN_LIGHT       A7 //measures ambiant light

// The following two pins are shared, assuming stepper and solenoid
// won't be used at the same time. So, make sure when using stepper
// motor the voltage supply is off
#define PIN_STEP        A1 //stepper step
#define PIN_SOL_EN      A1 //solenoid enable

#define PIN_STEP_SLP    A2 //stepper sleep, active low
#define PIN_VREG_IN     A3 //measures voltage of VREG output
#define PIN_BL_CS       A4  //backlight(APA102C) chip select
#define PIN_SHD_SLP     A5  //shade driver sleep, active low
#define PIN_STEP_STOP   2 //stepper stopper input
#define PIN_SHD_DRV     3  //shade driver
#define PIN_MT_DIR      A0 //shade motor direction 
#define PIN_MT_DIR2     4  //shade motor direction inverted
#define PIN_MT_EN       5  //shade motor enable
#define PIN_PWR_SEL     6  //power select between shade and motor
#define PIN_VREG_EN     7  //VREG enable
#define PIN_SRV2        8  //servo 2 (eyepiece servo) pwm
#define PIN_SRV1        9  //servo 1 (mirror servo) pwm
#define PIN_POT_CS      10 //VREG digital pot chip select, active low

// The following pins are multiplexed among pot SPI, backlight SPI and stepper.
// When STEP_SLP is off, i.e. when stepper is active, make sure both
// BACKLIGHT_CS and POT_CS are deselected.
#define PIN_MOSI        11 //atmega328 SPI master output
#define PIN_STEP_DIR    11 //stepper direction
#define PIN_BL_DIN      11 //backlight data in
#define PIN_POT_DIN     11 //VREG digitial pot data in
#define PIN_SOL_DIR     11 //solenoid direction
#define PIN_MISO        12 //atmega328 SPI master input
#define PIN_POT_DOUT    12 //VREG digital pot data out
#define PIN_SCLK        13 //atmega328 SPI clock output
#define PIN_POT_CLK     13 //VREG digital pot clock in
#define PIN_BL_CLK      13 //backlight clock input
////////////////////////////////////////////////////////////

#define DECLARE_TIMEOUT(_p) static unsigned long _##_p##_start, _##_p##_tick
#define _RESET_TIMEOUT(_p,_ticks) _##_p##_start=_ticks()

// cheap timeout detection, when wrap around, the actual timeout may double
#define _IS_TIMEOUT(_p,_timeout,_ticks) \
    (_##_p##_tick=_ticks(),\
     (_##_p##_tick>=_##_p##_start&&(_##_p##_tick-_##_p##_start)>=_timeout) || \
        (_##_p##_tick<_##_p##_start&&_##_p##_tick>=_timeout))

// accurate timeout detection
#define _IS_TIMEOUT2(_p,_timeout,_ticks) \
    (_##_p##_tick=_ticks(),\
     (_##_p##_tick>=_##_p##_start&&(_##_p##_tick-_##_p##_start)>=_timeout) || \
        (_##_p##_tick<_##_p##_start&&((0xffffffff-_##_p##_start)+_##_p##_tick)>=_timeout))

#define UPDATE_TIMEOUT(_p) _##_p##_start = _##_p##_tick

#define RESET_TIMEOUT(_p) _RESET_TIMEOUT(_p,millis)
#define IS_TIMEOUT(_p,_timeout) _IS_TIMEOUT(_p,_timeout,millis)
#define IS_TIMEOUT2(_p,_timeout) _IS_TIMEOUT2(_p,_timeout,millis)

#define RESET_TIMEOUT_MS(_p) _RESET_TIMEOUT(_p,micros)
#define IS_TIMEOUT_MS(_p,_timeout) _IS_TIMEOUT(_p,_timeout,micros)
#define IS_TIMEOUT2_MS(_p,_timeout) _IS_TIMEOUT2(_p,_timeout,micros)

#define VOLTAGE_4v5 185
#define VOLTAGE_5v 195

#include "shell.h"
#include "stepper.h"
#include "vreg.h"
#include "shade.h"
#include "solenoid.h"
#include "sv.h"
#include "led.h"

void setup() {
    pinMode(PIN_LIGHT,INPUT);

    digitalWrite(PIN_STEP_SLP,LOW);
    pinMode(PIN_STEP_SLP,OUTPUT);

    digitalWrite(PIN_VREG_EN,LOW);
    pinMode(PIN_VREG_EN,OUTPUT);

    pinMode(PIN_MT_DIR,INPUT);
    pinMode(PIN_MT_DIR2,INPUT);
    digitalWrite(PIN_MT_EN,LOW);
    pinMode(PIN_MT_EN,OUTPUT);

    pinMode(PIN_VREG_IN,INPUT);

    digitalWrite(PIN_PWR_SEL,LOW);
    pinMode(PIN_PWR_SEL,OUTPUT);

    pinMode(PIN_SHD_DRV,INPUT);
    pinMode(PIN_STEP_STOP,INPUT_PULLUP);

    digitalWrite(PIN_POT_CS,HIGH);
    pinMode(PIN_POT_CS,OUTPUT);
    
    digitalWrite(PIN_BL_CS,LOW);
    pinMode(PIN_BL_CS,OUTPUT);

    digitalWrite(PIN_SHD_SLP,LOW);
    pinMode(PIN_SHD_SLP,OUTPUT);

    digitalWrite(PIN_SOL_EN,LOW);
    pinMode(PIN_SOL_EN,OUTPUT);

    pinMode(PIN_STEP,OUTPUT);

    setupShell();
    setupVr();
    setupShade();
    setupServo();
    setupSolenoid();
    setupStepper();
    setupLed();
}

void loop() {
    loopShell();
    loopVr();
    loopShade();
    loopSolenoid();
    loopServo();
    loopStepper();
    loopLed();
}

