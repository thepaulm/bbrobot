#include "nervous_system.h"

nervous_system *spine;

nervous_system::nervous_system(arm *rf, arm *lf, arm *rb, arm *lb)
: right_arm(rf)
, left_arm(lf)
, right_leg(rb)
, left_leg(lb)
, state(0)
{
}

nervous_system::~nervous_system()
{
}

void
nervous_system::connect()
{
    if (right_arm) right_arm->connect();
    if (left_arm) left_arm->connect();
    if (right_leg) right_leg->connect();
    if (left_leg) left_leg->connect();
}

void
nervous_system::disconnect()
{
    if (right_arm) right_arm->disconnect();
    if (left_arm) left_arm->disconnect();
    if (right_leg) right_leg->disconnect();
    if (left_leg) left_leg->disconnect();
}

bool
nervous_system::arms_busy()
{
    return (left_arm->busy() || right_arm->busy() ||
            left_leg->busy() || right_leg->busy());
}

void
nervous_system::walking(scheduler *sched)
{
    if (arms_busy())
        return;

    switch (state) {
        case 1:
            {
                right_arm->cycle_forward(sched, this);
                left_leg->cycle_forward(sched, this);
                state = 2;
            }
            break;

        case 2:
            {
                right_arm->cycle_backward(sched, this);
                left_arm->cycle_forward(sched, this);

                right_leg->cycle_forward(sched, this);
                left_leg->cycle_backward(sched, this);

                state = 3;
            }
            break;

        case 3:
            {
                right_arm->cycle_forward(sched, this);
                left_arm->cycle_backward(sched, this);

                right_leg->cycle_backward(sched, this);
                left_leg->cycle_forward(sched, this);
                state = 2;
            }
            break;

        case 86:
            stop(sched);
            break;
    }
}

/* arm_completion_handler callback */
void
nervous_system::finish(scheduler *sched, arm *done)
{
    /* there are no outstanding arms doing movement, move to the next
       position */
    walking(sched);
}

void
nervous_system::walk(scheduler *sched)
{
    state = 1;
    walking(sched);
}

void
nervous_system::stop(scheduler *sched)
{
    if (arms_busy()) {
        state = 86;
    } else {
        right_arm->request_backward();
        left_arm->request_backward();
        right_leg->request_backward();
        left_leg->request_backward();

        right_arm->request_down();
        left_arm->request_down();
        right_leg->request_down();
        left_leg->request_down();

        state = 0;
    }
}
