#ifndef HMO_MOTOR_INCLUDED
#define HMO_MOTOR_INCLUDED

class HmoMotor {
private:
    HmoTimer t_;
    byte state_;
    const byte pinEnable_;
    const byte pinDir_;
    const byte pinDir2_;

protected:
    virtual void powerSetup(byte value)=0;

    void disable() {
        if(state_){
            pinMode(pinDir_,INPUT);
            pinMode(pinDir2_,INPUT);
            digitalWrite(pinEnable_,LOW);
            pinMode(pinEnable_,OUTPUT);
            state_ = 0;
        }
    }

    void dirve(bool dir) {
        //turn off shade first
        disable();
        digitalWrite(pinDir_,dir?HIGH:LOW);
        digitalWrite(pinDir2_,dir?LOW:HIGH);
        if(!state_) {
            pinMode(pinEnable_,INPUT_PULLUP);
            pinMode(pinDir_,OUTPUT);
            pinMode(pinDir2_,OUTPUT);
            powerSetup(MT_VOLTAGE);
            t_.reset();
            state_ = dir?1:2;
        }
    }

public:
    HmoMotor(byte pinEnable,byte pinDir,byte pinDir2)
        :state_(0)
        ,pinEnable_(pinEnable)
        ,pinDir_(pinDir)
        ,pinDir2_(pinDir2)
    {}

    numvar cmd() {
        byte n = getarg(0);
        if(!n)
            disable();
        else
            dirve(!!getarg(1));
        return 0;
    }

    void setup() {
        pinMode(pinDir_,INPUT);
        pinMode(pinDir2_,INPUT);
        digitalWrite(pinEnable_,LOW);
        pinMode(pinEnable_,OUTPUT);
    }

    void loop() {
        if(!state_) return;
        // Shade motor will run until any of the two stop
        // switch is triggered. We use a timeout to account
        // for possible jamming situation
        if(t_.timeout(MT_TIMEOUT)) {
            shellReply("!");
            disable();
        }else if(digitalRead(pinEnable_) == LOW)
            disable();
    }
};
#endif// HMO_MOTOR_INCLUDED
