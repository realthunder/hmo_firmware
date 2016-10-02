#ifndef HMO_INCLUDED
#define HMO_INCLUDED

#include <Arduino.h>
#include <bitlash.h>

class HmoTimer {
private:
    uint32_t start_;
    uint32_t ticks_;

    // cheap timeout detection, when wrap around, the actual timeout may double
    bool isTimeout(uint32_t t) const{
        return (ticks_>=start_&&(ticks_-start_)>=t) ||
            (ticks_<start_&&ticks_>=t);
    }
    // accurate timeout detection
    bool isTimeout2(uint32_t t) const{
        return (ticks_>=start_&&(ticks_-start_)>=t) || 
            (ticks_<start_&&((0xffffffff-start_)+ticks_)>=t);
    }

public:
    void update() {
        start_ = ticks_;
    }

    void reset() {
        start_ = millis();
    }
    bool timeout(uint32_t t) {
        ticks_ = millis();
        return isTimeout(t);
    }
    bool timeout2(uint32_t t) {
        ticks_ = millis();
        return isTimeout2(t);
    }

    void resetUs() {
        start_ = micros();
    }
    bool timeoutUs(uint32_t t) {
        ticks_ = micros();
        return isTimeout(t);
    }
    bool timeout2Us(uint32_t t) {
        ticks_ = micros();
        return isTimeout2(t);
    }
};

class HmoModule {
private:
    HmoModule *next_;
public:
    HmoModule()
        :next_(0)
    {}

    void link(HmoModule *next) {
        next_ = next;
    }
    HmoModule *next() {
        return next_;
    }

    virtual void setup() = 0;
    virtual void loop() = 0;
};

void shellReply(const char *c);
byte shellID();
void hmoRegister(HmoModule *m);

#endif
