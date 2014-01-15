#ifndef _NERVOUS_SYSTEM_H
#define _NERVOUS_SYSTEM_H

#include "arm.h"

class nervous_system : public arm_completion_handler, public schedule_item
{
public:
    nervous_system(arm *lf, gpio *lfvalve,
                   arm *rf, gpio *rfvalve,
                   arm *lb, gpio *lbvalve,
                   arm *rb, gpio *rbvalve,
                   gpio *pump);
    ~nervous_system();

    void connect();
    void disconnect();

    void walk(scheduler *);
    void stop(scheduler *);

    void pump_left();
    void pump_right();
    void pump_both();

    void finish(scheduler *, arm *);
    void fire(scheduler *);

    arm *right_arm;
    arm *left_arm;
    arm *right_leg;
    arm *left_leg;

    gpio *pump;

    gpio *lfvalve;
    gpio *rfvalve;
    gpio *lbvalve;
    gpio *rbvalve;

    int state;

private:
    void walking(scheduler *);

    bool arms_busy();
};

extern nervous_system *spine;
#endif /* _NERVOUS_SYSTEM_H */
