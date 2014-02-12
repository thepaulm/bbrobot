#ifndef _NERVOUS_SYSTEM_H
#define _NERVOUS_SYSTEM_H

#include "arm.h"
#include "threaded_control.h"

class nervous_system : public threaded_control
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

    void halt_walk(scheduler *);
    void walk(scheduler *);

    void pump_left();
    void pump_right();
    void pump_both();

    void control(threaded_control_mgr *);

    arm *right_arm;
    arm *left_arm;
    arm *right_leg;
    arm *left_leg;

    gpio *pump;

    gpio *lfvalve;
    gpio *rfvalve;
    gpio *lbvalve;
    gpio *rbvalve;

private:
    threaded_control_mgr *thr_con;
    bool halting;
};

extern nervous_system *spine;
#endif /* _NERVOUS_SYSTEM_H */
