#ifndef HMO_STEPPER_INCLUDED
#define HMO_STEPPER_INCLUDED

class HmoStepper {
private:
    HmoTimer t_;
    volatile int counter_;
    int pos_;
    unsigned delay_;
    unsigned overshoot_;
    byte toggle_;
    byte dir_;
    const byte pinDir_;
    const byte pinStep_;

protected:
    virtual void powerSetup(byte value)=0;

public:
    HmoStepper(byte pinStep, byte pinDir, 
                unsigned delay=1000, 
                unsigned overshoot=10) 
        :counter_(0)
        ,pos_(0)
        ,delay_(delay)
        ,overshoot_(overshoot)
        ,toggle_(0)
        ,pinDir_(pinDir)
        ,pinStep_(pinStep)
    {}

    void disable() {
        if(counter_) {
            powerSetup(LOW);
            counter_ = 0;
        }
    }

    void step(int count) {
        if(counter_) {
            shellReply("!");
            return;
        }
        if(count==0) {
            pos_ = 0;
            return;
        }
        if(count>0) {
            digitalWrite(pinDir_,HIGH);
            dir_ = 1;
            counter_ = count;
        }else{
            digitalWrite(pinDir_,LOW);
            dir_ = -1;
            counter_ = -count;
        }
        t_.resetUs();
        toggle_ = 1;
        powerSetup(HIGH);
    }

    numvar cmd() {
        byte n = getarg(0);
        if(!n) {
            printInteger(pos_,0,0);
            if(counter_) {
                spb(',');
                printInteger(counter_,0,0);
            }
            speol();
        }else if(n==1)
            step(getarg(1));
        else switch(getarg(1)) {
        case 0:
            delay_ = getarg(2);
            break;
        case 1:
            overshoot_ = getarg(2);
            if(!overshoot_)
                overshoot_ = 1;
            break;
        }
        return 0;
    }

    void setup() {
        pinMode(pinDir_,OUTPUT);
        pinMode(pinStep_,OUTPUT);
    }

    void loop() {
        if(counter_ && t_.timeoutUs(delay_)) {
            t_.update();
            digitalWrite(pinStep_,toggle_);
            if(toggle_==0) {
                pos_ += dir_;
                if(counter_==1)
                    disable();
                else {
                    --counter_;
                    toggle_ = !toggle_;
                }
            }
        }
    }

    void stop() {
        if(counter_) 
            counter_ = overshoot_;
    }

    bool active() const {
        return counter_!=0;
    }
};

#endif //HMO_STEPPER_INCLUDED
