#if defined(HMO_EYEPIECE) || defined(HMO_AUTO)

#include "hmo.h"

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
//
#define VOLTAGE_4v5 185
#define VOLTAGE_5v 195

#include "stepper.h"
#include "vreg.h"
#include "shade.h"
#include "motor.h"
#include "solenoid.h"
#include "sv.h"

class Eyepiece: public HmoModule {
public:
    class EpStepper: public HmoStepper {
    protected:
        virtual void powerSetup(byte value) {
            if(value) digitalWrite(PIN_PWR_SEL,LOW);
            digitalWrite(PIN_STEP_SLP,value?HIGH:LOW);
        }
    public:
        EpStepper() 
            :HmoStepper(PIN_STEP,PIN_STEP_DIR)
        {}

        void setup() {
            digitalWrite(PIN_STEP_SLP,LOW);
            pinMode(PIN_STEP_SLP,OUTPUT);
            HmoStepper::setup();

            pinMode(PIN_STEP_STOP,INPUT_PULLUP);
            attachInterrupt(0,stStop,FALLING);
        }
    };
    static EpStepper st_;
    static numvar stCmd() {
        return st_.cmd();
    }
    static void stStop() {
        st_.stop();
    }


    class EpVReg: public HmoVReg {
    public:
        EpVReg()
            :HmoVReg(PIN_VREG_EN,PIN_VREG_IN,PIN_POT_CS)
        {}
        virtual void powerSetup(byte value) {
            //make sure stepper is deactivated because of SPI pins overload
            digitalWrite(PIN_STEP_SLP,LOW);
            HmoVReg::powerSetup(value);
        }
    };
    static EpVReg vr_;
    static numvar vrCmd() {
        return vr_.cmd();
    }


    class EpShade: public HmoShade {
    protected:
        virtual void powerSetup(byte value) {
            if(value && !HmoShade::active()) {
                //Shade shares the same voltage regulator with motor,
                //servo and solenoid. It may require voltage up to
                //9 volts. So we must turn off power selector for 
                //motor and solenoid
                digitalWrite(PIN_PWR_SEL,LOW);
            }
            Eyepiece::vr_.powerSetup(value);
        }
    public:
        EpShade()
            :HmoShade(PIN_SHD_DRV,PIN_SHD_SLP,PIN_LIGHT)
        {}
    };
    static EpShade shd_;
    static numvar shdCmd() {
        return shd_.cmd();
    }

    class EpMotor: public HmoMotor {
    protected:
        virtual void powerSetup(byte value) {
            Eyepiece::powerSetup(value);
        }
    public:
        EpMotor()
            :HmoMotor(PIN_MT_EN,PIN_MT_DIR,PIN_MT_DIR2)
        {}
    };
    static EpMotor mt_;
    static numvar mtCmd() {
        return mt_.cmd();
    }

    class EpSolenoid: public HmoSolenoid {
    protected:
        virtual void powerSetup(byte value) {
            Eyepiece::powerSetup(value);
        }

    public:
        EpSolenoid()
            :HmoSolenoid(PIN_SOL_EN,PIN_SOL_DIR)
        {}
    };
    static EpSolenoid sol_;

    static numvar solCmd() {
        return sol_.cmd();
    }

    class EpServoCtrl: public HmoServoCtrl {
    protected:
        virtual void powerSetup(byte value) {
            Eyepiece::powerSetup(value?VOLTAGE_4v5:0);
        }

    public:
        EpServoCtrl()
            :HmoServoCtrl(2)
        {
            HmoServoCtrl::init(0,PIN_SRV1);
            HmoServoCtrl::init(1,PIN_SRV2);
        }
    };
    static EpServoCtrl sv_;
    static numvar svCmd() {
        return sv_.cmd();
    }


    static void powerSetup(byte value) {
        if(value) {
            shd_.disable();
            vr_.powerSetup(value);
            delay(100);
            digitalWrite(PIN_PWR_SEL,HIGH);
        }else{
            vr_.powerSetup(0);
            digitalWrite(PIN_PWR_SEL,LOW);
        }
    }

public:
    Eyepiece() {
        hmoRegister(this);
    }

    bool active() {
#ifdef HMO_AUTO
        return shellID() && (shellID()&1);
#else
        return true;
#endif
    }

    virtual void setup() {
        if(!active()) return;

        digitalWrite(PIN_PWR_SEL,LOW);
        pinMode(PIN_PWR_SEL,OUTPUT);

        vr_.setup();
        addBitlashFunction("vr", vrCmd);

        shd_.setup();
        addBitlashFunction("shd", shdCmd);

        mt_.setup();
        addBitlashFunction("mt", mtCmd);

        sv_.setup();
        addBitlashFunction("sv", svCmd);

        sol_.setup();
        addBitlashFunction("sol", solCmd);

        st_.setup();
        addBitlashFunction("st", stCmd);
    }

    virtual void loop() {
        if(!active()) return;
        vr_.loop();
        shd_.loop();
        mt_.loop();
        sol_.loop();
        sv_.loop();
        st_.loop();
    }
};

Eyepiece::EpStepper Eyepiece::st_;
Eyepiece::EpServoCtrl Eyepiece::sv_;
Eyepiece::EpVReg Eyepiece::vr_;
Eyepiece::EpShade Eyepiece::shd_;
Eyepiece::EpMotor Eyepiece::mt_;
Eyepiece::EpSolenoid Eyepiece::sol_;
static Eyepiece ep;

#endif
