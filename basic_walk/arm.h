#ifndef _ARM_H
#define _ARM_H

#include "pwm.h"
#include "gpio.h"
#include "scheduler.h"
#include "config.h"

enum ARM_TYPE { LEFT = 1, RIGHT = 2, FRONT = 4, BACK = 8};

class arm;
class arm_sweep_delegate;

class arm_completion_handler
{
public:
    virtual void finish(scheduler *, arm *) = 0;
};

class arm : public schedule_item
{
public:
    arm(pwm *top, pwm *bottom,
        unsigned high_us, unsigned low_us,
        unsigned forward_us, unsigned backward_us, unsigned attach_us,
        int flags);
    ~arm();

    void connect();
    void disconnect();

    void schedule_fire(scheduler *);

    void sweep(scheduler *);

    /* XXXPAM: Add a ticket system so we can have multile oustanding */
    bool cycle_forward(scheduler *, arm_completion_handler *);
    bool cycle_backward(scheduler *, arm_completion_handler *);

    /* All of these return the pwm estimated transit time in us */
    int request_up();
    int request_down();
    int request_forward();
    int request_backward();
    int request_attach();
    int request_standing();

    /* Is this arm currently in motion */
    bool busy();

    void inc_top() {top->inc();}
    void inc_bottom() {bottom->inc();}
    void dec_top() {top->dec();}
    void dec_bottom() {bottom->dec();}

    void save_forward_state();
    void save_backward_state();
    void save_attach_state();
    void save_up_state();
    void save_down_state();

    struct config_arm *get_arm_config();
    bool use();

private:
    pwm *top;
    pwm *bottom;
    int flags;
    int state;

    unsigned high_us;
    unsigned low_us;
    unsigned forward_us;
    unsigned backward_us;
    unsigned attach_us;

    arm_completion_handler *comp;
    bool in_use;

friend class arm_sweep_delegate;
    arm_sweep_delegate *sweeper;
};

#endif /* _ARM_H */
