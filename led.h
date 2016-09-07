#ifndef HMO_LED_INCLUDED
#define HMO_LED_INCLUDED

#include <Adafruit_DotStar.h>

class HmoLed {
private:
    Adafruit_DotStar led_;
    const byte pinSelect_;
    bool active_;
    
protected:
    virtual void powerSetup(byte value)=0;

    void show(bool on=true) {
        if(!on) {
            if(!active_) return; 
            led_.clear();
            active_=false;
        } else if(!active_) {
            active_ = true;
            powerSetup(HIGH);
        }
        led_.begin();
        if(pinSelect_)
            digitalWrite(pinSelect_,HIGH);
        led_.show();
        if(pinSelect_)
            digitalWrite(pinSelect_,LOW);
        led_.end();
        if(!on) powerSetup(LOW);
    }

public:
    HmoLed(byte size, byte pinData, byte pinClock)
        :led_(size,pinData,pinClock)
        ,pinSelect_(0)
        ,active_(false)
    {}

    HmoLed(byte size, byte pinSelect)
        :led_(size)
        ,pinSelect_(pinSelect)
        ,active_(false)
    {}

    numvar cmd() {
        uint16_t i;
        byte r,g,b;
        byte n = getarg(0);
        if(!n) {
            show(false);
            return 0;
        }
        switch(getarg(1)) {
        case 0:
            show();
            break;
        case 1:
            if(n==3) 
                r = g = b = getarg(3);
            else {
                r = getarg(3);
                g = getarg(4);
                b = getarg(5);
            }
            led_.setPixelColor(getarg(2),r,g,b);
            break;
        case 2:
            if(n==2) 
                r = g = b = getarg(2);
            else {
                r = getarg(2);
                g = getarg(3);
                b = getarg(4);
            }
            for(i=0;i<led_.numPixels();++i)
                led_.setPixelColor(i,r,g,b);
            show();
            break;
        case 3:
            led_.updateLength(getarg(2));
            break;
        }
        return 0;
    }

    void setup() {
        if(pinSelect_) {
            digitalWrite(pinSelect_,LOW);
            pinMode(pinSelect_,OUTPUT);
        }
    }

    void loop() {
    }
};

#endif //HMO_LED_INCLUDED
