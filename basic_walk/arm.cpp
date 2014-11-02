#include <iostream>
#include "arm.h"
#include "arm_sweep_delegate.h"

using namespace std;


#define STATIONARY 0
#define MOVING_FORWARD 10
#define MOVING_BACKWARD 20

/* Arm */
arm::arm(pwm *top, pwm *bottom,
         unsigned high_us, unsigned low_us,
         unsigned forward_us, unsigned backward_us, unsigned attach_us,
         int flags)
: top(top)
, bottom(bottom)
, high_us(high_us)
, low_us(low_us)
, forward_us(forward_us)
, backward_us(backward_us)
, attach_us(attach_us)
, flags(flags)
, state(STATIONARY)
, comp(NULL)
, in_use(false)
{
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
    cout << "setting forward from " << forward_us;
    forward_us = bottom->get_duty_us();
    cout << " to " << forward_us << endl;
}

void
arm::save_backward_state()
{
    cout << "setting backward from " << backward_us;
    backward_us = bottom->get_duty_us();
    cout << " to " << backward_us << endl;
}

void
arm::save_attach_state()
{
    cout << "setting attach from " << attach_us;
    attach_us = bottom->get_duty_us();
    cout << " to " << attach_us << endl;
}

void
arm::save_up_state()
{
    cout << "setting up state from " << high_us;
    high_us = top->get_duty_us();
    cout << " to " << high_us << endl;
}

void
arm::save_down_state()
{
    cout << "setting down state from " << low_us;
    low_us = top->get_duty_us();
    cout << " to " << low_us << endl;
}

void
arm::connect()
{
    /* Set these to a neutral position half way through the sweep */
    request_standing();

    top->connect();
    bottom->connect();
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

bool
arm::use()
{
    bool old = in_use;
    in_use = true;
    return old;
}

/* Arm Specific Positions */

void
arm::schedule_fire(scheduler *sched)
{
    bool finish = false;
    int delay = 0;

    switch (state) {
        /* States for request forward:

           We bring the sholder up, bring the arm forward, bring the
           shoulder down */
        case MOVING_FORWARD:
            {
                delay = request_up();
                if (delay >= 0)
                    sched->add_schedule_item_ms(delay, this);
                else
                    cout << "Ooops, arm not scheduling" << endl;
            }
            break;

        case MOVING_FORWARD + 1:
            {
                delay = request_forward();
                if (delay >= 0)
                    sched->add_schedule_item_ms(delay, this);
                else
                    cout << "Ooops, arm not scheduling" << endl;
            }
            break;

        case MOVING_FORWARD + 2:
            {
                delay = request_down();
                if (delay >= 0)
                    sched->add_schedule_item_ms(delay, this);
                else
                    cout << "Ooops, arm not scheduling" << endl;
            }
            break;

        case MOVING_FORWARD + 3:
            {
                delay = request_attach();
                if (delay >= 0)
                    sched->add_schedule_item_ms(delay, this);
                else
                    cout << "Ooops, arm not scheduling" << endl;
            }
            break;

        case MOVING_FORWARD + 4:
            {
                finish = true;
            }
            break;


        /* States for request backward:

           We bring the arm backward */
        case MOVING_BACKWARD:
            {
                delay = request_backward();
                if (delay >= 0)
                    sched->add_schedule_item_ms(delay, this);
                else
                    cout << "Ooops, arm not scheduling" << endl;
            }
            break;

        case MOVING_BACKWARD + 1:
            {
                finish = true;
            }
            break;
    }


    if (finish) {
        arm_completion_handler *old_comp = comp;
        comp = NULL;
        state = STATIONARY;
        in_use = false;
        old_comp->finish(sched, this);
    } else {
        state ++;
    }
}

bool
arm::busy()
{
    return in_use;
}

/* XXXPAM: for all of these, make sure we don't have something
   pending before we accept new things */
bool
arm::cycle_forward(scheduler *sched, arm_completion_handler *c)
{
    if (comp) {
        cerr << "DOUBLE COMPLETION REQUEST FOR ARM" << endl;
        return false;
    }
    use();
    comp = c;
    state = MOVING_FORWARD;
    schedule_fire(sched);
}

bool
arm::cycle_backward(scheduler *sched, arm_completion_handler *c)
{
    if (comp) {
        cerr << "DOUBLE COMPLETION REQUEST FOR ARM" << endl;
        return false;
    }
    use();
    comp = c;
    state = MOVING_BACKWARD;
    schedule_fire(sched);
}

int
arm::request_forward()
{
    return bottom->set_duty_us(forward_us);
}

int
arm::request_backward()
{
    return bottom->set_duty_us(backward_us);
}

int
arm::request_standing()
{
    request_down();
    return bottom->set_duty_us((forward_us + backward_us) / 2);
}

int
arm::request_up()
{
    return top->set_duty_us(high_us);
}

int
arm::request_down()
{
    return top->set_duty_us(low_us);
}

int
arm::request_attach()
{
    return bottom->set_duty_us(attach_us);
}

struct config_arm *
arm::get_arm_config()
{
    struct config_arm *cfg = new struct config_arm;
    cfg->top = top;
    cfg->bottom = bottom;
    cfg->high_us = high_us;
    cfg->low_us = low_us;
    cfg->forward_us = forward_us;
    cfg->backward_us = backward_us;
    cfg->attach_us = attach_us;
    return cfg;
}

