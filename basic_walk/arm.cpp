#include <iostream>
#include "arm.h"
#include "arm_sweep_delegate.h"

#define LEG_HIGH 55
#define LEG_LOW 65
#define ARM_FORWARD 70
#define ARM_BACKWARD 30

using namespace std;

/* Arm */
arm::arm(pwm *top, pwm *bottom, int flags)
: top(top)
, bottom(bottom)
, flags(flags)
, state(0)
, comp(NULL)
{
    if (((flags & RIGHT) && (flags & FRONT)) ||
        ((flags & LEFT)  && (flags & BACK))) {
        high = 100 - LEG_HIGH;
        low = 100 - LEG_LOW;
    } else {
        high = LEG_HIGH;
        low = LEG_LOW;
    }

    if (flags & LEFT) {
        forward = ARM_FORWARD;
        backward = ARM_BACKWARD;
    } else {
        forward = 100 - ARM_FORWARD;
        backward = 100 - ARM_BACKWARD;
    }

    /* this guy is here to handle the fire commands for the sweep action */
    sweeper = new arm_sweep_delegate(this);
}

arm::~arm()
{
    delete top;
    delete bottom;
    delete sweeper;
}

void
arm::save_forward_state()
{
    cout << "setting forward from " << forward;
    forward = bottom->get_duty_pct();
    cout << " to " << forward << endl;
}

void
arm::save_backward_state()
{
    cout << "setting backward from " << backward;
    backward = bottom->get_duty_pct();
    cout << " to " << backward << endl;
}

void
arm::save_up_state()
{
    cout << "setting up state from " << high;
    high = top->get_duty_pct();
    cout << " to " << high << endl;
}

void
arm::save_down_state()
{
    cout << "setting down state from " << low;
    low = top->get_duty_pct();
    cout << " to " << low << endl;
}

void
arm::connect()
{
    top->connect();
    bottom->connect();

    request_backward();
    request_down();
}

void
arm::disconnect()
{
    top->stop();
    bottom->stop();
}

void
arm::sweep(scheduler *sched)
{
    sweeper->sweep(sched);
}

/* Arm Specific Positions */

void
arm::fire(scheduler *sched)
{
    bool finish = false;
    int delay;

    switch (state) {
        /* States for request forward:

           We bring the sholder up, bring the arm forward, bring the
           shoulder down */
        case 10:
            {
                delay = request_up();
                if (delay > 0)
                    sched->add_schedule_item_ms(delay, this);
            }
            break;

        case 11:
            {
                delay = request_forward();
                if (delay > 0)
                    sched->add_schedule_item_ms(delay, this);
            }
            break;

        case 12:
            {
                delay = request_down();
                if (delay > 0)
                    sched->add_schedule_item_ms(delay, this);
            }
            break;

        case 13:
            {
                finish = true;
            }
            break;


        /* States for request backward:

           We bring the arm backward */
        case 20:
            {
                delay = request_backward();
                if (delay > 0)
                    sched->add_schedule_item_ms(delay, this);
            }
            break;

        case 21:
            {
                finish = true;
            }
            break;
    }

    if (finish) {
        arm_completion_handler *old_comp = comp;
        comp = NULL;
        state = 0;
        old_comp->finish(sched, this);
    } else {
        state ++;
    }
}

bool
arm::busy()
{
    return state != 0;
}

/* XXXPAM: for all of these, make sure we don't have something
   pending before we accept new things */
bool
arm::cycle_forward(scheduler *sched, arm_completion_handler *c)
{
    if (comp)
        return false;
    comp = c;
    state = 10;
    fire(sched);
}

bool
arm::cycle_backward(scheduler *sched, arm_completion_handler *c)
{
    if (comp)
        return false;
    comp = c;
    state = 20;
    fire(sched);
}

int
arm::request_forward()
{
    return bottom->set_duty_pct(forward);
}

int
arm::request_backward()
{
    return bottom->set_duty_pct(backward);
}

int
arm::request_up()
{
    return top->set_duty_pct(high);
}

int
arm::request_down()
{
    return top->set_duty_pct(low);
}
