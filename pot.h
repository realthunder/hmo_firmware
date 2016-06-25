#include <FrequencyTimer2.h>
#include <SPI.h>
#include "mcp4xxx.h"

using namespace icecave::arduino;
MCP4XXX pot;
word vreg_en;

void vregEnable(byte enable) {
    if(enable == 255) {
        //force off
        if(vreg_en) {
            digitalWrite(PIN_VREG_EN,LOW);
            vreg_en = 0;
        }
        return;
    }
    if(enable) {
        if(++vreg_en == 1) {
            digitalWrite(PIN_VREG_EN,HIGH);
        }
    }else if(vreg_en && --vreg_en == 0) 
        digitalWrite(PIN_VREG_EN,LOW);
}

numvar potCmd() {
    byte n = getarg(0);
    if(!n) {
        int v = analogRead(PIN_VREG_IN);
        printIntegerInBase(v,10,3,0);
        sp(',');
        v = (150*analogRead(PIN_VREG_IN))<<10;
        printIntegerInBase(v,10,3,0);
        speol();
        return 0;
    }
    switch(getarg(1)) {
    case 0:
        pot.set(getarg(2));
        break;
    }
    return 0;
}
void setupPot() {
    addBitlashFunction("pot", (bitlash_function) potCmd);
}

void loopPot() {
}

