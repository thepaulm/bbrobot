#ifndef _NERVOUS_SYSTEM_H
#define _NERVOUS_SYSTEM_H

#include "arm.h"

class nervous_system : public arm_completion_handler
{
public:
    nervous_system(arm *rf, arm *lf, arm *rb, arm *lb);
    ~nervous_system();

    void connect();
    void disconnect();

    void walk(scheduler *);
    void stop(scheduler *);

    void finish(scheduler *, arm *);

    arm *right_arm;
    arm *left_arm;
    arm *right_leg;
    arm *left_leg;

    int state;

private:
    void walking(scheduler *);

    bool arms_busy();
};

extern nervous_system *spine;
#endif /* _NERVOUS_SYSTEM_H */
