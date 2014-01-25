#ifndef _THREADED_CONTROL_H
#define _THREADED_CONTROL_H

#include <set>

#include <pthread.h>
#include "arm.h"

class threaded_control_mgr;

class threaded_control
{
public:
    virtual void control(threaded_control_mgr *) = 0;

};

class threaded_control_mgr : public arm_completion_handler,
                             public schedule_item,
                             public io_item
{
public:
    threaded_control_mgr();
    ~threaded_control_mgr(); 

    /* Callbacks from the ios */
    void finish(scheduler *, arm *);
    void schedule_fire(scheduler *); 
    void io_fire(scheduler *);

    /* Services for the threaded control */
    void wait_arms(arm *arms[], int count);
    void wait_schedule_item(unsigned us);
    int wait_keypress();

    void arm_cycle_forward(scheduler *sched, arm *);
    void arm_cycle_backward(scheduler *sched, arm *);

    /* This is how you invoke the thread */
    void start_controller(threaded_control *);
    void controller_done(threaded_control *);

private:
    void wait_condvar();
    void wait_iovar();

    pthread_cond_t condvar;
    pthread_mutex_t condvarmut;    
    bool condvar_triggered;

    pthread_cond_t iovar;
    pthread_mutex_t iovarmut;
    bool iovar_triggered;

    pthread_cond_t armsvar;
    pthread_mutex_t armsmut;

    int char_to_deliver;
    std::set<arm *>waiting_arms;
};

#endif /* _THREADED_CONTROL_H */
