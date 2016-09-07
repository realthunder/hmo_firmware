#if defined(HMO_EYEPIECE) || defined(HMO_AUTO)

#include "hmo.h"
#include <Adafruit_MCP23017.h>
#include "stepper.h"
#include "sv.h"
#include "fan.h"
#include "tmp.h"
#include "led.h"

#define PIN_SRV_LOCK    A0
#define PIN_LED_DAT     A1
#define PIN_LED_CLK     A2
#define PIN_TMP         A3
#define PIN_INTA        2
#define PIN_INTB        3
#define PIN_STEP        4
#define PIN_STEP_DIR    5
#define PIN_IO_RST      6
#define PIN_BAT2_STAT   7
#define PIN_BAT1_STAT   8
#define PIN_BAT_CTL     9
#define PIN_SRV_BRAKE   10
#define PIN_FAN_PWM     11
#define PIN_SRV_FOOT    12
#define PIN_SRV_CAM     13
#define PIN_BAT2_VOUT   A6
#define PIN_BAT1_VOUT   A7

#define PIN_EP_EN      0
#define PIN_LED_EN     1
#define PIN_FRAME_STOP 2
#define PIN_STEP_STOP1 3
#define PIN_STEP_STOP2 4
#define PIN_SRV_EN     5
#define PIN_FAN_EN     6
#define PIN_FAN_TACHO  7
#define PIN_BTN23      8
#define PIN_BTN22      9
#define PIN_BTN21      10
#define PIN_STEP_SLP   11
#define PIN_HOST_INT   12
#define PIN_BTN11      13
#define PIN_BTN12      14
#define PIN_BTN13      15

#define IO_ADDR 7

class Body: public HmoModule {
private:
    static Adafruit_MCP23017 mcp_;
    static bool interruptPending_;
    static bool buttonPending_;
    static byte button_;

public:
    Body() 
    {
        hmoRegister(this);
    }

    static void stepperInterrupt(bool on) {
        if(on) {
            mcp_.setupInterruptPin(PIN_FRAME_STOP,FALLING);
            mcp_.setupInterruptPin(PIN_STEP_STOP1,FALLING);
            mcp_.setupInterruptPin(PIN_STEP_STOP2,FALLING);
            if(fan_.tachoActive())
                mcp_.setupInterruptPin(PIN_FAN_TACHO,LOW);
        }else{
            mcp_.setupInterruptPin(PIN_FRAME_STOP,LOW);
            mcp_.setupInterruptPin(PIN_STEP_STOP1,LOW);
            mcp_.setupInterruptPin(PIN_STEP_STOP2,LOW);
            if(fan_.tachoActive())
                mcp_.setupInterruptPin(PIN_FAN_TACHO,RISING);
        }
    }

    static void fanInterrupt(bool on) {
        if(!on || !st_.active())
            mcp_.setupInterruptPin(PIN_FAN_TACHO,on?RISING:LOW);
    }

    static void powerSetup(byte pin, byte value) {
        mcp_.digitalWrite(pin,value?HIGH:LOW);
    }

    static void ioSetup() {
        digitalWrite(PIN_IO_RST,LOW);
        delay(100);
        digitalWrite(PIN_IO_RST,HIGH);
        delay(100);

        mcp_.begin(IO_ADDR);
        mcp_.pinMode(PIN_EP_EN,OUTPUT);
        mcp_.pinMode(PIN_LED_EN,OUTPUT);
        mcp_.pinMode(PIN_SRV_EN,OUTPUT);
        mcp_.pinMode(PIN_FAN_EN,OUTPUT);
        mcp_.pinMode(PIN_FAN_TACHO,INPUT);
        mcp_.pinMode(PIN_HOST_INT,OUTPUT);
        mcp_.pinMode(PIN_STEP_SLP,OUTPUT);

        mcp_.pullUp(PIN_FRAME_STOP,HIGH);
        mcp_.pullUp(PIN_STEP_STOP1,HIGH);
        mcp_.pullUp(PIN_STEP_STOP2,HIGH);
        mcp_.pullUp(PIN_BTN11,HIGH);
        mcp_.pullUp(PIN_BTN12,HIGH);
        mcp_.pullUp(PIN_BTN13,HIGH);
        mcp_.pullUp(PIN_BTN21,HIGH);
        mcp_.pullUp(PIN_BTN22,HIGH);
        mcp_.pullUp(PIN_BTN23,HIGH);
        mcp_.setupInterruptPin(PIN_BTN11,FALLING);
        mcp_.setupInterruptPin(PIN_BTN12,FALLING);
        mcp_.setupInterruptPin(PIN_BTN13,FALLING);
        mcp_.setupInterruptPin(PIN_BTN21,FALLING);
        mcp_.setupInterruptPin(PIN_BTN22,FALLING);
        mcp_.setupInterruptPin(PIN_BTN23,FALLING);
        mcp_.setupInterrupts(false,false,LOW);

        if(st_.active())
            stepperInterrupt(true);
        if(fan_.tachoActive())
            fanInterrupt(true);
    }

    static numvar ioCmd() {
        byte n = getarg(0);
        if(!n) {
            ioSetup();
        }else if(n==1) {
            switch(getarg(1)) {
            case 0:
                n = Body::mcp_.readGPIO(0);
#define PRINT_BIT(_b) (n&(1<<_b))?spb('1'):spb('0')
                PRINT_BIT(PIN_FRAME_STOP);
                PRINT_BIT(PIN_STEP_STOP1);
                PRINT_BIT(PIN_STEP_STOP2);
                speol();
                break;
            case 1:
                n = Body::mcp_.readGPIO(1);
                printHex(n);
                speol();
                break;
            case 2:
                printHex(button_);
                speol();
                button_ = 0xff;
                break;
            }
        }else{
            switch(getarg(1)) {
            case 0:
                powerSetup(PIN_EP_EN,getarg(1));
                break;
            }
        }
        return 0;
    }


    class BodyServoCtrl: public HmoServoCtrl {
    protected:
        virtual void powerSetup(byte value) {
            Body::powerSetup(PIN_SRV_EN,value);
        }
    public:
        BodyServoCtrl()
            :HmoServoCtrl(4)
        {
            HmoServoCtrl::init(0,PIN_SRV_BRAKE);
            HmoServoCtrl::init(0,PIN_SRV_LOCK);
            HmoServoCtrl::init(0,PIN_SRV_CAM);
            HmoServoCtrl::init(0,PIN_SRV_FOOT);
        }
    };
    static BodyServoCtrl sv_;
    static numvar svCmd() {
        return sv_.cmd();
    }

    class BodyStepper: public HmoStepper {
    protected:
        virtual void powerSetup(byte value) {
            Body::powerSetup(PIN_STEP_SLP,value);
            Body::stepperInterrupt(!!value);
        }
    public:
        BodyStepper()
            :HmoStepper(PIN_STEP, PIN_STEP_DIR)
        {}
        void setup() {
            digitalWrite(PIN_STEP_SLP,LOW);
            pinMode(PIN_STEP_SLP,OUTPUT);
            HmoStepper::setup();
        }
    };
    static BodyStepper st_;
    static numvar stCmd() {
        return st_.cmd();
    }


    class BodyFan: public HmoFan {
    protected:
        virtual void powerSetup(byte value) {
            Body::powerSetup(PIN_FAN_EN,value);
        }
        virtual void tachoSetup(bool on) {
            Body::fanInterrupt(on);
        }
    public:
        BodyFan()
            :HmoFan(PIN_FAN_PWM)
        {}
    };
    static BodyFan fan_;
    static numvar fanCmd() {
        return fan_.cmd();
    }


    class BodyLed: public HmoLed {
    protected:
        virtual void powerSetup(byte value) {
            Body::powerSetup(PIN_LED_EN,value);
        }
    public:
        BodyLed()
            :HmoLed(HMO_LED,PIN_LED_DAT,PIN_LED_CLK)
        {}
    };
    static BodyLed led_;
    static numvar ledCmd() {
        return led_.cmd();
    }


    class BodyTmp: public HmoTmp {
    protected:
        virtual void powerSetup(byte value) {
        }
    public:
        BodyTmp()
            :HmoTmp(PIN_TMP)
        {}
    };
    static BodyTmp tmp_;
    static numvar tmpCmd() {
        return tmp_.cmd();
    }


    bool active() {
#ifdef HMO_AUTO
        return shellID() && !(shellID()&1);
#else
        return true;
#endif
    }

    static void onInterrupt() {
        interruptPending_ = true;
    }

    static void onButton() {
        buttonPending_ = true;
    }

    virtual void setup() {
        if(!active()) return;

        pinMode(PIN_IO_RST,OUTPUT);
        ioSetup();
        addBitlashFunction("io", ioCmd);

        sv_.setup();
        addBitlashFunction("sv", svCmd);

        st_.setup();
        addBitlashFunction("st", stCmd);

        fan_.setup();
        addBitlashFunction("fan", fanCmd);

        led_.setup();
        addBitlashFunction("led", ledCmd);

        tmp_.setup();
        addBitlashFunction("tmp", tmpCmd);

        // attachInterrupt(1,Body::onInterrupt,FALLING);
        // attachInterrupt(1,Body::onButton,FALLING);
    }

    virtual void loop() {
        if(!active()) return;

        if(interruptPending_ || buttonPending_) {
            uint16_t v = mcp_.readGPIOAB();

            if(interruptPending_) {
                interruptPending_ = false;
                byte n = v&0xff;
                if(st_.active() && 
                    (n&((1<<PIN_FRAME_STOP)|(1<<PIN_STEP_STOP1)|(1<<PIN_STEP_STOP2))))
                    st_.stop();
                if(fan_.tachoActive() && (n&(1<<PIN_FAN_TACHO)))
                    fan_.tachoCount();
            }
            if(buttonPending_) {
                buttonPending_ = false;
                button_ &= v>>8;
            }
        }

        sv_.loop();
        st_.loop();
        fan_.loop();
        led_.loop();
        tmp_.loop();
    }

};

Adafruit_MCP23017 Body::mcp_;
bool Body::interruptPending_;
bool Body::buttonPending_;
byte Body::button_;
Body::BodyServoCtrl Body::sv_;
Body::BodyStepper Body::st_;
Body::BodyFan Body::fan_;
Body::BodyTmp Body::tmp_;
Body::BodyLed Body::led_;
static Body body;

#endif
