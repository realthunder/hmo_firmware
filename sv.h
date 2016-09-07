#ifndef HMO_SV_INCLUDED
#define HMO_SV_INCLUDED

#include <Servo.h>

//HK-5320 servo range 24~164
//
#define SV_INTERVAL 2000

class HmoServoCtrl {
private:
    struct HmoServo {
        byte pin;
        unsigned interval;
        HmoTimer t_;
        Servo ctrl;
        byte active;

        HmoServo() 
            :pin(0)
            ,active(0)
        {}
    };

    HmoServo *sv_;
    byte count_;
    byte activeCount_;

protected:
    virtual void powerSetup(byte value) = 0;

    void disable(byte index) {
        if(index>=count_){
            shellReply("!");
            return;
        }
        if(sv_[index].active) {
            sv_[index].ctrl.detach();
            sv_[index].active = 0;
            if(--activeCount_ == 0)
                powerSetup(LOW);
        }
    }

    void drive(byte index, byte value, unsigned interval) {
        if(index>=count_){
            shellReply("!");
            return;
        }
        if(!sv_[index].active) {
            if(++activeCount_ == 1)
                powerSetup(HIGH);
            sv_[index].ctrl.attach(sv_[index].pin);
            sv_[index].active = 1;
        }
        sv_[index].ctrl.write(value);
        if((sv_[index].interval = interval))
            sv_[index].t_.reset();
    }

    void init(byte index, byte pin) {
        if(index<count_)
            sv_[index].pin = pin;
    }

public:
    HmoServoCtrl(byte count)
        :count_(count)
        ,activeCount_(0)
    {
        sv_ = new HmoServo[count];
    }

    virtual ~HmoServoCtrl() {
        delete[] sv_;
    }

    numvar cmd() {
        byte n = getarg(0);
        if(count_ == 1) {
            if(!n)
                disable(0);
            else
                drive(0,getarg(1),n==2?getarg(2):SV_INTERVAL);
        }else if(!n) {
            byte i;
            for(i=0;i<count_;++i)
                disable(i);
        }else if(n==1)
            disable(getarg(1));
        else
            drive(getarg(1),getarg(2),n==3?getarg(3):SV_INTERVAL);
        return 0;
    }

    void setup() {}

    void loop() {
        if(!activeCount_) return;
        byte i;
        for(i=0;i<count_;++i) {
            if(sv_[i].active && 
                sv_[i].interval && 
                sv_[i].t_.timeout(sv_[i].interval))
                disable(i);
        }
    }

    byte count() const {
        return count_;
    }
};

#endif
