#include <Adafruit_DotStar.h>
#include <SPI.h>

#define LED_DEFAULT_LENGTH 9
Adafruit_DotStar led(LED_DEFAULT_LENGTH);

void ledShow() {
    //make sure stepper is deactivated because of SPI pins overload
    stDisable();
    led.begin();
    digitalWrite(PIN_BL_CS,HIGH);
    led.show();
    digitalWrite(PIN_BL_CS,LOW);
    led.end();
}

numvar ledCmd() {
    uint16_t i;
    byte r,g,b;
    byte n = getarg(0);
    if(!n) {
        ledShow();
        return 0;
    }
    switch(getarg(1)) {
    case 0:
        led.updateLength(getarg(2));
        break;
    case 1:
        if(n==3) 
            r = g = b = getarg(3);
        else {
            r = getarg(3);
            g = getarg(4);
            b = getarg(5);
        }
        led.setPixelColor(getarg(2),r,g,b);
        break;
    case 2:
        if(n==2) 
            r = g = b = getarg(2);
        else {
            r = getarg(2);
            g = getarg(3);
            b = getarg(4);
        }
        for(i=0;i<led.numPixels();++i)
            led.setPixelColor(i,r,g,b);
        ledShow();
        break;
    }
    return 0;
}

void setupLed() {
    addBitlashFunction("led", (bitlash_function) ledCmd);
    led.clear();
    ledShow();
}

void loopLed() {
}
