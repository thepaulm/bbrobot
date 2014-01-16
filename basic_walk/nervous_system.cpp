#include "nervous_system.h"
#include "stdin_handler.h"

#define PUMP_SWITCH_DELAY_MS 1000

nervous_system *spine;

nervous_system::nervous_system(arm *lf, gpio *lfvalve,
                               arm *rf, gpio *rfvalve,
                               arm *lb, gpio *lbvalve,
                               arm *rb, gpio *rbvalve,
                               gpio *pump)
: left_arm(lf)
, lfvalve(lfvalve)

, right_arm(rf)
, rfvalve(rfvalve)

, left_leg(lb)
, lbvalve(lbvalve)

, right_leg(rb)
, rbvalve(rbvalve)

, pump(pump)
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
    if (pump) pump->off();
    if (lfvalve) lfvalve->off();
    if (rfvalve) rfvalve->off();
    if (lbvalve) lbvalve->off();
    if (rbvalve) rbvalve->off();
}

bool
nervous_system::arms_busy()
{
    return (left_arm->busy() || right_arm->busy() ||
            left_leg->busy() || right_leg->busy());
}

/* Valves fire across the body. Normally they are either rf+lb, or lf+rb. This
   is the way that the plumbing is hooked up. The front valves control the
   negative, and the back valves control the positive for the OPPOSITE side.

   rf + lb = right front and left back are negative pressure, left front and
             right back are positive pressure.
   lf + rb = left front and right back are negative pressure, right front and
             left back are positive pressure.

   If you do both fronts on then everybody is at negative pressure. If you
   do both backs on then everybody is positive pressure. Do not maintain
   these situations for a long time as you will overwhelm the pump. */ 

void
nervous_system::pump_left()
{
    lfvalve->on();
    rbvalve->on();

    rfvalve->off();
    lbvalve->off();
}

void
nervous_system::pump_right()
{
    rfvalve->on();
    lbvalve->on();

    lfvalve->off();
    rbvalve->off();
}

void
nervous_system::pump_both()
{
    lfvalve->on();
    rfvalve->on();
    lbvalve->off();
    rbvalve->off();
}

#define PUMP_BOTH 1

void
nervous_system::walking(scheduler *sched)
{
    if (arms_busy())
        return;

    switch (state) {
        case 1:
            {
                pause_for_key();
                pump->on();
                right_arm->cycle_forward(sched, this);
                left_leg->cycle_forward(sched, this);
                state = 2;
            }
            break;

        case 2:
            {
                pause_for_key();
#if PUMP_BOTH
                pump_both();
#else
                pump_right();
#endif
                state = 3;
                sched->add_schedule_item_ms(PUMP_SWITCH_DELAY_MS, this);
            }
            break;

        case 3:
            {
                pause_for_key();
                pump_right();
                right_arm->cycle_backward(sched, this);
                left_arm->cycle_forward(sched, this);

                right_leg->cycle_forward(sched, this);
                left_leg->cycle_backward(sched, this);

                state = 4;
            }
            break;

        case 4:
            {
                pause_for_key();
#if PUMP_BOTH
                pump_both();
#else
                pump_left();
#endif
                state = 5;
                sched->add_schedule_item_ms(PUMP_SWITCH_DELAY_MS, this);
            }
            break;

        case 5:
            {
                pause_for_key();
                pump_left();
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

/* schedule_item callback */
/* These calls are always just pump delay completions */
void
nervous_system::fire(scheduler *sched)
{
    walking(sched);
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

        pump->off();
        usleep(200 * 1000);
        lfvalve->off();
        rfvalve->off();
        lbvalve->off();
        rbvalve->off();
        state = 0;
    }
}
