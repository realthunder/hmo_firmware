#include <FrequencyTimer2.h>
#include <SPI.h>
#include "mcp4xxx.h"

using namespace icecave::arduino;

void vrSet(word value) {
    static MCP4XXX pot(PIN_POT_CS);
    if(!value) {
        digitalWrite(PIN_VREG_EN,LOW);
        return;
    }

    //make sure stepper is deactivated because of SPI pins overload
    stDisable();
    pot.set(value);
    digitalWrite(PIN_VREG_EN,HIGH);
}

numvar vrCmd() {
    if(getarg(0)){ 
        vrSet(getarg(1));
        return 0;
    }
    uint32_t v = analogRead(PIN_VREG_IN);
    printIntegerInBase(v,10,3,0);
    spb(',');

    // We are using 10K in a 10K+20K divider to measure voltage on a 5V device.
    // This gives us in units of milli-volt
    printIntegerInBase((5000*3*v)>>10,10,3,0);
    speol();
    return 0;
}

void setupVr() {
    addBitlashFunction("vr", (bitlash_function) vrCmd);
}

void loopVr() {
}

