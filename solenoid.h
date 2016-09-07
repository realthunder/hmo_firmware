#ifndef HMO_SOLENOID_INCLUDED
#define HMO_SOLENOID_INCLUDED

class HmoSolenoid {
private:
    HmoTimer t_;
    unsigned long interval_;
    const byte pinEnable_;
    const byte pinDir_;
    byte active_;
    byte volt_;

protected:
    virtual void powerSetup(byte value) = 0;

    void drive(byte dir) {
        if(!active_) {
            digitalWrite(pinDir_,dir?HIGH:LOW);
            powerSetup(volt_);
            digitalWrite(pinEnable_,HIGH);
            t_.reset();
            active_ = 1;
        }
    }

public:
    HmoSolenoid(byte pinEnable,byte pinDir)
        :interval_(500)
        ,pinEnable_(pinEnable)
        ,pinDir_(pinDir)
        ,active_(0)
        ,volt_(VOLTAGE_4v5)
    {}

    void disable() {
        if(active_){
            digitalWrite(pinEnable_,0);
            powerSetup(LOW);
            active_ = 0;
        }
    }

    numvar cmd() {
        byte n = getarg(0);
        if(!n)
            disable();
        else if(n==1)
            drive(getarg(1));
        else {
            switch(getarg(1)) {
            case 2:
                volt_ = getarg(2);
                break;
            case 3:
                interval_ = getarg(2);
                break;
            }
        }
        return 0;
    }

    void setup() {
        digitalWrite(pinEnable_,LOW);
        pinMode(pinEnable_,OUTPUT);
    }

    void loop() {
        if(active_ && t_.timeout(interval_))
            disable();
    }
};

#endif // HMO_SOLENOID_INCLUDED
