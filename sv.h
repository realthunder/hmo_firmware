#include <Servo.h>

#define INIT_TIMEOUT unsigned long _t_start = millis(), _t_tick;
#define RESET_TIMEOUT _t_start=millis()

// cheap timeout detection, when wrap around, the actual timeout may double
#define IS_TIMEOUT(_timeout) \
    (_t_tick=millis(),\
     (_t_tick>=_t_start&&(_t_tick-_t_start)>=_timeout) || \
        (_t_tick<_t_start&&_t_tick>=_timeout))

// accurate timeout detection
#define IS_TIMEOUT2(_timeout) \
    (_t_tick=millis(),\
     (_t_tick>=_t_start&&(_t_tick-_t_start)>=_timeout) || \
        (_t_tick<_t_start&&((0xffffffff-_t_start)+_t_tick)>=_timeout))

int servo_pos;
int servo_step;
int servo_max = 180;
Servo myservo;   

INIT_TIMEOUT;

void loopServo() {
 if(!servo_step || !IS_TIMEOUT2(servo_step)) return;
  if(servo_pos > 0) {
      myservo.write(servo_pos);
      if(++servo_pos >= servo_max) servo_pos = -servo_max;
  }else{
      myservo.write(-servo_pos);
      ++servo_pos;
  }
  RESET_TIMEOUT;
}

int sv(unsigned n, unsigned arg1, unsigned arg2) {
    servo_step = 0;
    if(!n) {
        myservo.detach();
        return 0;
    }
    switch(arg1) {
    case 1:
        servo_step = n<2?15:arg2;
        if(!servo_step)
            myservo.detach();
        else {
            myservo.attach(SERVO_PIN);
            RESET_TIMEOUT;
        }
        break;
    case 2:
        servo_pos = n<2?90:arg2;
        myservo.attach(SERVO_PIN);
        myservo.write(servo_pos);
        delay(10000);
        myservo.detach();
        break;
    case 3:
        servo_max = n<2?180:arg2;
        break;
    default:
        return -1;
    }

    sp("servo step: ");
    printInteger(servo_step,0,0);
    sp(", pos: ");
    printInteger(servo_pos,0,0);
    speol();
    return 0;
}

numvar svCmd() {
    return sv(getarg(0),getarg(1),getarg(2));
}

void setupServo() {
    addBitlashFunction("sv", (bitlash_function) svCmd);
}
