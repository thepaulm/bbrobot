#include <iostream>
#include "arm_sweep_delegate.h"

using namespace std;

arm_sweep_delegate::arm_sweep_delegate(arm *arm)
: parm(arm)
, state(0)
{
}

void
arm_sweep_delegate::fire(scheduler *sched)
{
    int delay;
    switch (state) {
        case 1:
        {
            cout << "leg up" << endl;
            delay = parm->request_up();
            if (delay > 0)
                sched->add_schedule_item_ms(delay, this);
        }
        break;

        case 2:
        {
            cout << "arm forward" << endl;
            delay = parm->request_forward();
            if (delay > 0)
                sched->add_schedule_item_ms(delay, this);
        }
        break;

        case 3:
        {
            cout << "leg down" << endl;
            delay = parm->request_down();
            if (delay > 0)
                sched->add_schedule_item_ms(delay, this);
        }
        break;

        case 4:
        {
            cout << "arm backward" << endl;
            delay = parm->request_backward();
            if (delay > 0)
                sched->add_schedule_item_ms(delay, this);
        }
        break;

        case 5:
        {
            cout << "arm is done" << endl;
            state = -1;
        }
        break;

        default:
        {
            state = -1;
        }
        break;
    }
    state++;
}

void
arm_sweep_delegate::sweep(scheduler *sched)
{
    state = 1;
    fire(sched);
}
