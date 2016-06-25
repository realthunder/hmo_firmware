#define MT_VOLTAGE 0xf0
#define MT_TIMEOUT 3000
DECLEAR_TIMEOUT(shd);

byte shdActive;
byte shdAuto;
byte shdMotorState;

void shdMotorDrive(byte dir) {
    switch(dir) {
    case 0:
        if(shdMotorState){
            digitalWrite(PIN_PWR_SEL,LOW);
            vregEnable(0);
            shdMotorState = 0;
        }
        break;
    case 2:
    case 1:
        digitalWrite(PIN_MT_DIR,dir==1?HIGH:LOW);
        if(!shdMotorState) {
            pot.set(MT_VOLTAGE);
            vregEnable(1);
            digitalWrite(PIN_PWR_SEL,HIGH);
            RESET_TIMEOUT(shd);
            shdMotorState = dir;
        }
        break;
    }
}

void shdEnable(byte enable) {
    if(enable) {
        if(!shdActive) {
            //turn off power for motor and solenoid
            digitalWrite(PIN_PWR_SEL,LOW);
            vregEnable(1);
            FrequencyTimer2::setPeriod(10000);
            FrequencyTimer2::enable();
            pinMode(PIN_SHD_DRV,OUTPUT);
            //put stepper to sleep, and this also enables shade dirver
            digitalWrite(PIN_STEP_SLP,HIGH);
            shdActive = 1;
        }
    }else if(shdActive){
        vregEnable(0);
        FrequencyTimer2::disable();
        pinMode(PIN_SHD_DRV,INPUT);
        shdActive = 0;
    }
}

numvar shdCmd() {
    switch(getarg(1)) {
    case 0:
        shdEnable(1);
        break;
    case 1:
        shdEnable(0);
        break;
    case 2:
        shdAuto = 1;
        break;
    case 3:
        shdAuto = 0;
        break;
    case 4:
        // print ambiant light reading
        printIntegerInBase(analogRead(PIN_LIGHT),10,3,0);
        speol();
        break;
    case 5:
        shdMotorDrive(0);
        break;
    case 6:
        shdMotorDrive(1);
        break;
    case 7:
        shdMotorDrive(2);
        break;
    }
    return 0;
}

void setupShd() {
    addBitlashFunction("shd", (bitlash_function) shdCmd);
    shdEnable(0);
}

void loopShd() {
    if(shdMotorState) {
        if(IS_TIMEOUT(shd,MT_TIMEOUT)) {
            sp('!');
            speol();
            shdMotorDrive(0);
        }else if(digitRead(PIN_MT_STOP) == LOW)
            shdMotorDrive(0);
    }
}

