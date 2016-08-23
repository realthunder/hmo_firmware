#define SV_VOLTAGE VOLTAGE_4v5

//HK-5320 servo range 24~164

#include <Servo.h>

byte svActive;
unsigned svInterval = 2000;
Servo sv;   
DECLARE_TIMEOUT(sv);
static const byte svPins[2] = {PIN_SRV1,PIN_SRV2};

void svDisable() {
    if(svActive) {
        vrSet(0);
        sv.detach();
        svActive = 0;
    }
}

void svDrive(byte index, byte value) {
    if(index>=sizeof(svPins))
        shellReply("1");
    else if(svActive)
        shellReply("2");
    else {
        shdDisable();
        vrSet(SV_VOLTAGE);
        digitalWrite(PIN_PWR_SEL,HIGH);
        sv.attach(svPins[index]);
        sv.write(value);
        RESET_TIMEOUT(sv);
        svActive = index+1;
    }
}

numvar svCmd() {
    byte n = getarg(0);
    if(!n)
        svDisable();
    else if(n==1)
        svInterval = getarg(1);
    else if(n==2)
        svDrive(getarg(1),getarg(2));
    else if(n==3) {
        solDrive(getarg(3));
        svDrive(getarg(1),getarg(2));
    }
    return 0;
}

void setupServo() {
    addBitlashFunction("sv", (bitlash_function) svCmd);
}

void loopServo() {
    if(svActive && IS_TIMEOUT(sv,svInterval))
        svDisable();
}

