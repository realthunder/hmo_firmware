#ifndef HMO_SHADE_INCLUDED
#define HMO_SHADE_INCLUDED

#include <FrequencyTimer2.h>

#define MT_VOLTAGE VOLTAGE_4v5
#define MT_TIMEOUT 3000
#define SHD_INTERVAL 10

class HmoShade {
private:
    HmoTimer t_;

    bool active_;
    byte value_;
    byte target_;

    const byte pinDrive_;
    const byte pinSleep_;
    const byte pinLight_;

protected:
    virtual void powerSetup(byte value)=0;

    void set(byte value) {
        if(!active_) {
            if(!value) return;
            FrequencyTimer2::setPeriod(10000);
            FrequencyTimer2::enable();
            pinMode(pinDrive_,OUTPUT);
            digitalWrite(pinSleep_,HIGH);
            value_ = value<=50?value:50;
            powerSetup(value_);
            //make sure to only mark active after powerSetup call
            active_ = true;
        }
        target_ = value;
        t_.reset();
    }

public:
    HmoShade(byte pinDrive,byte pinSleep,byte pinLight)
        :active_(false)
        ,value_(0)
        ,target_(0)
        ,pinDrive_(pinDrive)
        ,pinSleep_(pinSleep)
        ,pinLight_(pinLight)
    {}

    numvar cmd() {
        byte n = getarg(0);
        if(!n) 
            disable();
        else {
            switch(getarg(1)) {
            case 0:
                set(getarg(2));
                break;
            case 1:
                // print ambiant light reading
                printInteger(analogRead(pinLight_),0,0);
                speol();
                break;
            }
        }
        return 0;
    }

    void setup() {
        pinMode(pinLight_,INPUT);
        pinMode(pinDrive_,INPUT);
        digitalWrite(pinSleep_,LOW);
        pinMode(pinSleep_,OUTPUT);
    }

    void loop() {
        //gradrual change value to target
        if(active_ && value_!=target_) {
            if(t_.timeout(SHD_INTERVAL)) {
                if(value_ > target_)
                    powerSetup(--value_);
                else
                    powerSetup(++value_);
                t_.update();
            }
        }
    }

    void disable() {
        if(active_){
            digitalWrite(pinSleep_,LOW);
            FrequencyTimer2::disable();
            pinMode(pinDrive_,INPUT);
            powerSetup(0);
            active_ = false;
        }
    }

    bool active() const {
        return active_;
    }
};

#endif //HMO_SHADE_INCLUDED
