#ifndef HMO_VREG_INCLUDED
#define HMO_VREG_INCLUDED

#include <SPI.h>
#include "mcp4xxx.h"

using namespace icecave::arduino;

class HmoVReg {
private:
    MCP4XXX pot_;

    const byte pinEnable_;
    const byte pinInput_;
    const byte pinSelect_;

public:
    HmoVReg(byte pinEnable, byte pinInput, byte pinSelect)
        :pot_(pinSelect)
        ,pinEnable_(pinEnable)
        ,pinInput_(pinInput)
        ,pinSelect_(pinSelect)
    {}

    virtual void powerSetup(byte value) {
        if(!value) {
            digitalWrite(pinEnable_,LOW);
            return;
        }
        pot_.set(value);
        digitalWrite(pinEnable_,HIGH);
    }

    numvar cmd() {
        if(getarg(0)){ 
            powerSetup(getarg(1));
            return 0;
        }
        uint32_t v = analogRead(pinInput_);
        printInteger(v,0,0);
        spb(',');

        // We are using 10K in a 10K+20K divider to measure voltage on a 5V device.
        // This gives us in units of milli-volt
        printInteger((5000*3*v)>>10,0,0);
        speol();
        return 0;
    }

    void setup() {
        digitalWrite(pinEnable_,LOW);
        pinMode(pinEnable_,OUTPUT);
        pinMode(pinInput_,INPUT);

        digitalWrite(pinSelect_,HIGH);
        pinMode(pinSelect_,OUTPUT);
    }

    void loop() {}

};

#endif // HMO_VREG_INCLUDED
