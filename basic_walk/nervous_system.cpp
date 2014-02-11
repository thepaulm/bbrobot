#include "nervous_system.h"
#include "stdin_handler.h"

#define PUMP_SWITCH_DELAY_MS 1000

using namespace std;

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
{
    thr_con = new threaded_control_mgr();
}

nervous_system::~nervous_system()
{
    delete thr_con;
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
nervous_system::control(threaded_control_mgr *tcm)
{
    cout << "nervous_system::control starting ..." << endl;
    arm *arms[] = {right_arm, left_arm, right_leg, left_leg};

    /* XXX THe problem here is that we may not be on the completion
       list for these arms. We'll have to address this */

    /* wait until no movement */
    tcm->wait_arms(arms, 4);

    /* pump on for this whole thing */
    //pump->on();

    /* wait for key to start */
    cout << "------ PRE LOOP -----------------------------" << endl;
    if ('q' == tcm->wait_keypress()) goto done;

    /* initial forward position */
    tcm->arm_cycle_forward(sched, right_arm);
    tcm->arm_cycle_forward(sched, left_leg);

    while (true) {
        cout << "Waiting arms ..." << endl;
        tcm->wait_arms(arms, 4);
        cout << "1) All 4 arms DONE" << endl << endl;

        /* right_arm forward, switch to right arm */
        cout << "---------------------------------------------" << endl;
        cout.flush();
        if ('q' == tcm->wait_keypress()) goto done;
        cout << "pump both" << endl;
        pump_both();
        tcm->wait_schedule_item(PUMP_SWITCH_DELAY_MS); 

        /* switched. Now we only need pump on the right */
        cout << "---------------------------------------------" << endl;
        cout.flush();
        if ('q' == tcm->wait_keypress()) goto done;
        cout << "pump right" << endl;
        pump_right();

        /* walk with the right arm */
        cout << "---------------------------------------------" << endl;
        cout.flush();
        if ('q' == tcm->wait_keypress()) goto done;
        cout << "left walk" << endl;;
        tcm->arm_cycle_backward(sched, right_arm);
        tcm->arm_cycle_forward(sched, left_arm);

        tcm->arm_cycle_forward(sched, right_leg);
        tcm->arm_cycle_backward(sched, left_leg);
        tcm->wait_arms(arms, 4);
        cout << "2) All 4 arms DONE" << endl;

        /* left_arm forward. switch to left arm */
        cout << "---------------------------------------------" << endl;
        cout.flush();
        if ('q' == tcm->wait_keypress()) goto done;
        cout << "pump both" << endl;
        pump_both();
        tcm->wait_schedule_item(PUMP_SWITCH_DELAY_MS);

        /* switched. Now we only need pump on the left */
        cout << "---------------------------------------------" << endl;
        cout.flush();
        if ('q' == tcm->wait_keypress()) goto done;
        cout << "pump left" << endl;
        pump_left();

        /* Walk with the left arm */
        cout << "---------------------------------------------" << endl;
        cout.flush();
        if ('q' == tcm->wait_keypress()) goto done;
        cout << "right walk" << endl;
        tcm->arm_cycle_forward(sched, right_arm);
        tcm->arm_cycle_backward(sched, left_arm);

        tcm->arm_cycle_backward(sched, right_leg);
        tcm->arm_cycle_forward(sched, left_leg);
    }

done:
    pump->off();
    cout << "nervous_system::control finished." << endl;
    tcm->controller_done(this);
}

void
nervous_system::walk(scheduler *sched)
{
    thr_con->start_controller(this);
}

