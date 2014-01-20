#ifndef _ARM_SWEEP_DELEGATE_H
#define _ARM_SWEEP_DELEGATE_H

#include "arm.h"

class arm_sweep_delegate : public schedule_item
{
    friend class arm;
    arm_sweep_delegate(arm *arm);
    void sweep(scheduler *);

    void schedule_fire(scheduler *);

    arm *parm;
    int state;
};

#endif /* ARM_SWEEP_DELEGATE_H */

