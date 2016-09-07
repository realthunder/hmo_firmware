#ifndef HMO_FAN_INCLUDED
#define HMO_FAN_INCLUDED

#define TACHO_INTERVAL 5000
#define TACHO_RPM(_r)  (_r*60000/TACHO_INTERVAL)

class HmoFan {
private:
    HmoTimer t_;
    uint32_t tachoCount_;
    uint32_t tachoRPM_;
    byte tachoActive_;

    byte value_;
    const byte pinPwm_;
    bool active_;

protected:
    virtual void tachoSetup(bool on) = 0;
    virtual void powerSetup(byte value) = 0;

public:
    HmoFan(byte pinPwm)
        :tachoCount_(0)
        ,tachoRPM_(0)
        ,tachoActive_(0)
        ,value_(100)
        ,pinPwm_(pinPwm)
        ,active_(false)
    {}

    numvar cmd() {
        byte n = getarg(0);
        if(!n) {
            if(!tachoActive_) {
                tachoSetup(true);
                tachoActive_ = true;
                t_.reset();
            }
            tachoActive_ = 5;
            printInteger(tachoRPM_,0,0);
        }else if(n==1) {
            n = getarg(1);
            if(!n) {
                if(active_) {
                    if(pinPwm_) pinMode(pinPwm_,INPUT);
                    powerSetup(0);
                    active_ = false;
                }
            }else {
                value_ = n;
                if(pinPwm_)
                    analogWrite(pinPwm_,n);
                else
                    powerSetup(value_);
                if(!active_) {
                    if(pinPwm_) {
                        pinMode(pinPwm_,OUTPUT);
                        powerSetup(value_);
                    }
                    active_ = true;
                }
            }
        }
        return 0;
    }

    void tachoCount() {
        ++tachoCount_;
    }

    virtual void setup() {
        pinMode(pinPwm_,INPUT);
    }

    virtual void loop() {
        if(tachoActive_ && t_.timeout(TACHO_INTERVAL)) {
            t_.update();
            tachoRPM_ = TACHO_RPM(tachoCount_);
            tachoCount_ = 0;
            if(!--tachoActive_)
                tachoSetup(false);
        }
    }

    bool tachoActive() const {
        return tachoActive_;
    }

    bool active() const {
        return active_;
    }
};
#endif //HMO_FAN_INCLUDED
