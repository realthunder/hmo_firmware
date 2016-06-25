#define SOL_VOLTAGE 0xf0
//solenoid active time period in milliseconds
#define SOL_INTERVAL 500

byte solActive;
DECLEAR_TIMEOUT(sol);

void solDrive(byte dir) {
    switch(dir) {
    case 0:
        if(solActive){
            digitalWrite(PIN_SOL_EN,LOW);
            vregEnable(0);
            solActive = 0;
        }
        break;
    case 2:
    case 1:
        digitalWrite(PIN_SOL_DIR,dir==1?HIGH:LOW);
        if(!solActive) {
            pot.set(SOL_VOLTAGE);
            vregEnable(1);
            digitalWrite(PIN_PWR_SEL,HIGH);
            digitalWrite(PIN_SOL_EN,HIGH);
            RESET_TIMEOUT(sol);
            solActive = 1;
        }
        break;
    }
}

numvar solCmd() {
    solDrive(getarg(1));
    return 0;
}

void setupSol() {
    addBitlashFunction("sol", (bitlash_function) solCmd);
}

void loopSol() {
    if(solActive && IS_TIMEOUT(sol,SOL_INTERVAL))
        solEnable(0);
}
