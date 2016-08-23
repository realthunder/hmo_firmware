#define MT_VOLTAGE VOLTAGE_4v5
#define MT_TIMEOUT 3000
#define SHD_INTERVAL 10

DECLARE_TIMEOUT(shdmt);
DECLARE_TIMEOUT(shd);

byte shdActive;
byte shdAuto;
byte shdMotorState;
byte shdValue;
byte shdTargetValue;

void shdDisable() {
    if(shdActive){
        digitalWrite(PIN_SHD_SLP,LOW);
        FrequencyTimer2::disable();
        pinMode(PIN_SHD_DRV,INPUT);
        shdActive = 0;
        shdValue = 50;
    }
}

void shdSet(byte value) {
    if(!shdActive) {
        //turn off power for motor and solenoid
        digitalWrite(PIN_PWR_SEL,LOW);
        FrequencyTimer2::setPeriod(10000);
        FrequencyTimer2::enable();
        pinMode(PIN_SHD_DRV,OUTPUT);
        digitalWrite(PIN_SHD_SLP,HIGH);
        shdActive = 1;
        shdValue = 50;
    }
    shdTargetValue = value;
    RESET_TIMEOUT(shd);
}

void shdMotorDisable() {
    if(shdMotorState){
        digitalWrite(PIN_MT_EN,LOW);
        pinMode(PIN_MT_DIR,INPUT);
        pinMode(PIN_MT_DIR2,INPUT);
        pinMode(PIN_MT_EN,OUTPUT);
        shdMotorState = 0;
    }
}

void shdMotorDrive(byte dir) {
    //turn off shade first
    shdDisable();
    digitalWrite(PIN_MT_DIR,dir);
    digitalWrite(PIN_MT_DIR2,!dir);
    if(!shdMotorState) {
        vrSet(MT_VOLTAGE);
        pinMode(PIN_MT_EN,INPUT_PULLUP);
        pinMode(PIN_MT_DIR,OUTPUT);
        pinMode(PIN_MT_DIR2,OUTPUT);
        digitalWrite(PIN_PWR_SEL,HIGH);
        RESET_TIMEOUT(shdmt);
        shdMotorState = dir+1;
    }
}

numvar shdCmd() {
    byte n = getarg(0);
    if(!n) 
        shdDisable();
    else {
        switch(getarg(1)) {
        case 0:
            shdSet(getarg(2));
            break;
        case 1:
            // print ambiant light reading
            printIntegerInBase(analogRead(PIN_LIGHT),10,3,0);
            speol();
            break;
        case 2:
            shdMotorDisable();
            break;
        case 3:
            shdMotorDrive(0);
            break;
        case 4:
            shdMotorDrive(1);
            break;
        }
    }
    return 0;
}

void setupShade() {
    addBitlashFunction("shd", (bitlash_function) shdCmd);
    shdDisable();
}

void loopShade() {
    if(shdMotorState) {
        // Shade motor will run until any of the two stop
        // switch is triggered. We use a timeout to account
        // for possible jamming situation
        if(IS_TIMEOUT(shdmt,MT_TIMEOUT)) {
            shellReply("!");
            shdMotorDisable();
        }else if(digitalRead(PIN_MT_EN) == LOW)
            shdMotorDisable();
    }else if(shdActive && shdValue!=shdTargetValue) {
        if(IS_TIMEOUT(shd,SHD_INTERVAL)) {
            if(shdValue > shdTargetValue)
                vrSet(--shdValue);
            else
                vrSet(++shdValue);
            UPDATE_TIMEOUT(shd);
        }
    }
}

