unsigned stepCount;
unsigned stepDelay;

int st(unsigned dir, unsigned count, unsigned delay) {
    digitalWrite(STEPPER_DIR,dir==1?HIGH:LOW);

    stepDelay = delay;
    stepCount = count;

    printInteger(dir,0,0);
    sp(",");
    printInteger(stepCount,0,0);
    sp(",");
    printInteger(stepDelay,0,0);
    speol();

    digitalWrite(STEPPER_SLP,stepCount?HIGH:LOW);
    return 0;
}

numvar stCmd() {
    return st(getarg(1),getarg(2),getarg(3));
}

void loopSt() {
    if(!stepCount) return;
    digitalWrite(STEPPER_STEP,HIGH);
    delay(stepDelay);
    digitalWrite(STEPPER_STEP,LOW);
    delay(stepDelay);
    if(--stepCount == 0)
        digitalWrite(STEPPER_SLP,LOW);
}

void setupSt() {
    pinMode(STEPPER_DIR,OUTPUT);
    pinMode(STEPPER_STEP,OUTPUT);
    digitalWrite(STEPPER_SLP,LOW);
    pinMode(STEPPER_SLP,OUTPUT);
    addBitlashFunction("st", (bitlash_function) stCmd);
}
