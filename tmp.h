#ifndef HMO_TMP_INCLUDED
#define HMO_TMP_INCLUDED

class HmoTmp {
private:
    HmoTimer t_;
    unsigned long interval_;
#define TMP_STATE_IDLE 0
#define TMP_STATE_ACTIVE 1
#define TMP_STATE_PENDING 2
#define TMP_DELAY 1000
    byte state_;
    const byte pin_;

protected:
    virtual void powerSetup(byte value) = 0;

public:
    HmoTmp(byte pin)
        :interval_(5000)
        ,state_(TMP_STATE_IDLE)
        ,pin_(pin)
    {}

    numvar cmd() {
        byte n = getarg(0);
        if(!n) {
            t_.reset();
            if(state_!=TMP_STATE_IDLE) {
                powerSetup(HIGH);
                state_ = TMP_STATE_PENDING;
            }
            if(state_!=TMP_STATE_ACTIVE)
                spb('0');
            else {
                uint16_t v = (analogRead(pin_)*5000)>>10;
                printInteger((v-500)/10,0,0);
            }
            speol();
        }else if(n==2) {
            switch(getarg(1)) {
            case 0:
                interval_ = getarg(1);
                break;
            }
        }
        return 0;
    }

    void setup() {
        pinMode(pin_,INPUT);
    }

    void loop() {
        switch(state_){
        case TMP_STATE_IDLE:
            return;
        case TMP_STATE_PENDING:
            if(!t_.timeout(TMP_DELAY)) return;
            t_.update();
            state_ = TMP_STATE_ACTIVE;
            //fall through
        case TMP_STATE_ACTIVE:
            if(!t_.timeout(interval_)) return;
            powerSetup(LOW);
            state_ = TMP_STATE_IDLE;
            break;
        }
    }
};
#endif //HMO_TMP_INCLUDED
