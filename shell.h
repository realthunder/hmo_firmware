#ifdef SHELL_EEPROM
#include <EEPROM.h>
#endif

#ifdef SHELL_REBOOT
#include <avr/wdt.h>
#endif

#include "version.h"

#ifndef SHELL_BAUDRATE
#define SHELL_BAUDRATE 9600
#endif

#ifndef SHELL_BROADCAST
#   define SHELL_BROADCAST '0'
#endif
static byte shID;
static byte shState;
static byte shWakeupKey[4];

byte BITLASH_TXEN = 1;

// Hardware dependent code to enable/disable UART TX pin.
// The code below works for most Atmega chip. 
// Check the hardware/arduino/cores/arduino/HardwareSerial.cpp
// or equivalant for the register setting.
// If you want to use RS485, here is the place to toggle the 
// transmitter enable pin.
void enableTX(byte enable) {
#ifndef SHELL_TRACE
    if(enable) {
        bitSet(UCSR0B, TXEN0);
        BITLASH_TXEN = 1;
    } else {
        Serial.flush();
        bitClear(UCSR0B, TXEN0);
        pinMode(1,INPUT);
        BITLASH_TXEN = 0;
    }
#endif
}

void shellReply(const char *c) {
    sp(c);
    speol();
}

#ifdef SHELL_TRACE
#   define SHELL_SETSTATE(_s) do{\
        spb(shState+'0');\
        spb('-');\
        spb('>');\
        spb(_s+'0');\
        speol();\
        shState = _s;\
    }while(0)
#else
#   define SHELL_SETSTATE(_s) shState = _s
#endif
    
numvar idCmd() {
    byte n = getarg(0);
    if(shState==5) {//we received a broadcast id command
        if(!n){
            // If no argument, increase the current ID, intended for quick
            // test of broadcast feature.
            ++shID; 
        }else switch(getarg(1)){
        case 0:
            // If the boradcast command has the first arguemnt as 0, then force print
            // out the current ID. This is in case we forgot the ID. Make sure
            // only a single device is online before doing this.
            enableTX(1);
            break;
        case 1:
            // Sleep command to help online firmware upgrade
            // For all device except the one with id in getarg(2), ignore all data 
            // received from UART until receiving the wakeup key given in next 4 
            // arguments
            if(shID != getarg(2)) {
                shWakeupKey[0] = getarg(3);
                shWakeupKey[1] = getarg(4);
                shWakeupKey[2] = getarg(5);
                shWakeupKey[3] = getarg(6);
                SHELL_SETSTATE(6);
            }
            return 0;
        }
    }else if(n) { //If has any argument, set id, and start state machine
        shID = getarg(1);
        //None-zero shID sets the state to 1, while zero id
        //sets the state to 0 to bypass the state machine
        SHELL_SETSTATE(!!shID);

#ifdef SHELL_EEPROM
        if(n>1) {
            switch(getarg(2)) {
            case 0:
                EEPROM.write(0,shID);
                break;
            case 1:
                printHex(EEPROM.read(0));
                speol();
                break;
            }
        }
#endif
    }
    //print out current ID
    if(!shID) {
        sp("nil");
    }else
        spb(shID);
    spb(' ');
    sp(HMO_VERSION);
    speol();
    return 0;
}

#ifdef SHELL_REBOOT
numvar rebootCmd() {
    // Now ask the watchdog to reset us
    wdt_enable(WDTO_250MS);
    while(1);
    return 0;
}
#endif

void setupShell() {
#ifdef SHELL_EEPROM
    shID = EEPROM.read(0);
    if(shID == 0xff) shID = 0;
    if(shID) {
        // BITLASH_TXEN = 0;
        SHELL_SETSTATE(2);
    }
#endif

    initBitlash(SHELL_BAUDRATE);
    //delayed TX disable, because initBitlash() enables the serial TX
    if(!BITLASH_TXEN)
        enableTX(0);

    addBitlashFunction("id", (bitlash_function) idCmd);

#ifdef SHELL_REBOOT
    addBitlashFunction("reboot", (bitlash_function) rebootCmd);
#endif

}

void loopShell() {
    char c;
    if(!Serial.available()) return;
    switch(shState) {
    case 1://just finished one command, disabling UART TX
        enableTX(0);
        SHELL_SETSTATE(2);
        //fall through
    case 2://checking for ID
        c = Serial.read();
        if(c == SHELL_BROADCAST) {
            //Got broadcast command, calling bitlash without txen set
            SHELL_SETSTATE(5);
        }else if(c == shID)  {
            //ID found, enable UART TX
            enableTX(1);
            SHELL_SETSTATE(4);
        }else{
            SHELL_SETSTATE(3);//skipping till eol
            goto NEXT;
        }
        break;
    case 3://skiping till eol
        c = Serial.read();
NEXT:
        if(c == '\r' || c == '\n')
            SHELL_SETSTATE(2);
        break;
    case 5://receiving broadcast command
    case 4://waiting for command eol 
        c = Serial.peek();
        if(c == '\r' || c == '\n') {
            runBitlash();
            if(shState!=6)
                SHELL_SETSTATE(2);
            enableTX(0);
            break;
        }
        // fall through
    case 0://hand over to bitlash
        runBitlash();
        break;

    case 6://sleeping, checking for wakeup key
    case 7:
    case 8:
    case 9:
        c = Serial.read();
        if(c == shWakeupKey[shState-6]) {
            if(shState == 9) //wakeup
                SHELL_SETSTATE(2);
            else
                SHELL_SETSTATE(shState+1);
        }else// NOTE! not dealing with repetitive bytes in the key here,
             // so better not use keys with repetitive bytes.
            SHELL_SETSTATE(6);
        break;
    }
}

byte shellID() {
    return shID;
}

