volatile unsigned stepCount;
unsigned stepDelay = 1000;
unsigned stepOvershoot = 10;
byte stToggle;
DECLARE_TIMEOUT(st);

void stDisable() {
    if(stepCount) {
        stepCount = 0;
        digitalWrite(PIN_STEP_SLP,LOW);
    }
}

void st(unsigned dir, unsigned count) {
    if(stepCount) {
        shellReply("!");
        return;
    }
    digitalWrite(PIN_PWR_SEL,LOW);
    digitalWrite(PIN_STEP_DIR,dir==1?HIGH:LOW);
    stepCount = count;
    RESET_TIMEOUT_MS(st);
    stToggle = 1;
    digitalWrite(PIN_STEP_SLP,HIGH);
}

numvar stCmd() {
    byte n = getarg(0);
    if(!n) {
        printIntegerInBase(stepCount,10,3,0);
        speol();
    }else switch(getarg(1)) {
    case 0:
    case 1:
        st(getarg(1),getarg(2));
        break;
    case 2:
        stepDelay = getarg(2);
        break;
    case 3:
        stepOvershoot = getarg(2);
        if(!stepOvershoot)
            stepOvershoot = 1;
        break;
    }
    return 0;
}

void loopStepper() {
    if(stepCount && IS_TIMEOUT2_MS(st,stepDelay)) {
        UPDATE_TIMEOUT(st);
        digitalWrite(PIN_STEP,stToggle);
        if(stToggle==0 && --stepCount==0)
            digitalWrite(PIN_STEP_SLP,LOW);
        stToggle = !stToggle;
    }
}

void stStop() {
    if(stepCount) 
        stepCount = stepOvershoot;
}

void setupStepper() {
    addBitlashFunction("st", (bitlash_function) stCmd);
    attachInterrupt(0,stStop,FALLING);
}
