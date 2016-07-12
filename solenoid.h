byte solActive;
byte solVolt = VOLTAGE_4v5;
unsigned solInterval = 500;
DECLARE_TIMEOUT(sol);

void solDisable() {
    if(solActive){
        digitalWrite(PIN_SOL_EN,LOW);
        solActive = 0;
    }
}

void solDrive(byte dir) {
    if(!solActive) {
        shdDisable();
        vrSet(solVolt);
        digitalWrite(PIN_SOL_EN,HIGH);
        digitalWrite(PIN_SOL_DIR,dir?HIGH:LOW);
        digitalWrite(PIN_PWR_SEL,HIGH);
        RESET_TIMEOUT(sol);
        solActive = 1;
    }
}

numvar solCmd() {
    byte n = getarg(0);
    if(!n)
        solDisable();
    else if(n==1)
        solDrive(getarg(1));
    else {
        switch(getarg(1)) {
        case 2:
            solVolt = getarg(2);
            break;
        case 3:
            solInterval = getarg(2);
            break;
        }
    }
    return 0;
}

void setupSolenoid() {
    addBitlashFunction("sol", (bitlash_function) solCmd);
}

void loopSolenoid() {
    if(solActive && IS_TIMEOUT(sol,solInterval))
        solDisable();
}
