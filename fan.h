#ifndef HMO_FAN_INCLUDED
#define HMO_FAN_INCLUDED

#define FAN_PWM_MIN 50
#define FAN_TACHO_INTERVAL 2000
#define FAN_PWM_INTERVAL 50
#define FAN_TACHO_RPM(_r)  (_r*30000/FAN_TACHO_INTERVAL)

class HmoFan {
private:
    HmoTimer t_;
    HmoTimer tValue_;
    uint32_t tachoCount_;
    uint32_t tachoValue_;
    byte tachoActive_;

    byte current_;
    byte value_;
    const byte pinPwm_;
    bool active_;

protected:
    virtual void tachoSetup(bool on) = 0;
    virtual void powerSetup(byte value) = 0;

public:
    HmoFan(byte pinPwm)
        :tachoCount_(0)
        ,tachoValue_(0)
        ,tachoActive_(0)
        ,current_(0)
        ,value_(0)
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
            printInteger(FAN_TACHO_RPM(tachoValue_),0,0);
        }else if(n==1) {
            n = getarg(1);
            if(!n) {
                if(active_) {
                    if(pinPwm_) analogWrite(pinPwm_,0);
                    powerSetup(0);
                    active_ = false;
                    current_ = value_ = 0;
                }
            }else if(n!=value_ && n>FAN_PWM_MIN) {
                if(!active_) {
                    if(pinPwm_)
                        powerSetup(1);
                    active_ = true;
                    current_ = FAN_PWM_MIN;
                }
                tValue_.reset();
                value_ = n;
            }
        }
        return 0;
    }

    void tachoCount() {
        ++tachoCount_;
    }

    virtual void setup() {
        analogWrite(pinPwm_,0);
        pinMode(pinPwm_,OUTPUT);
        //change timer 2 frequency to 31KHz, works on Atmega328
        TCCR2B = (TCCR2B & 0b11111000) | 1;
    }

    virtual void loop() {
        if(value_!=current_ && tValue_.timeout(FAN_PWM_INTERVAL)) {
            tValue_.update();
            if(current_<value_)
                ++current_;
            else
                --current_;
            if(pinPwm_)
                analogWrite(pinPwm_,current_);
            else
                powerSetup(current_);
        }
        if(tachoActive_ && t_.timeout(FAN_TACHO_INTERVAL)) {
            t_.update();
            tachoValue_ = tachoCount_;
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
