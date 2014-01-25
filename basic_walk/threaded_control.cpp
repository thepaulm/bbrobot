#include <functional>

#include <pthread.h>
#include "threaded_control.h"
#include "scheduler.h"
#include "stdin_handler.h"

using namespace std;

threaded_control_mgr::threaded_control_mgr()
: condvar_triggered(false)
, iovar_triggered(false)
{
    if (0 != pthread_cond_init(&condvar, NULL)) {
        cerr << "pthread_cond_init failed." << endl;
    }
    if (0 != pthread_mutex_init(&condvarmut, NULL)) {
        cerr << "pthread_mutex_init failed." << endl;
    }
    if (0 != pthread_cond_init(&iovar, NULL)) {
        cerr << "pthread_cond_init failed." << endl;
    }
    if (0 != pthread_mutex_init(&iovarmut, NULL)) {
        cerr << "pthread_mutex_init failed." << endl;
    }
    if (0 != pthread_cond_init(&armsvar, NULL)) {
        cerr << "pthread_cond_init failed." << endl;
    }
    if (0 != pthread_mutex_init(&armsmut, NULL)) {
        cerr << "pthread_mutex_init failed." << endl;
    }
}

threaded_control_mgr::~threaded_control_mgr()
{
    pthread_cond_destroy(&condvar);
    pthread_mutex_destroy(&condvarmut);
    pthread_cond_destroy(&iovar);
    pthread_mutex_destroy(&iovarmut);
    pthread_cond_destroy(&armsvar);
    pthread_mutex_destroy(&armsmut);
}

/* --------------------------------
   System Upcalls
   --------------------------------*/
void
threaded_control_mgr::finish(scheduler *sched, arm *arm)
{
    bool release = false;
    pthread_mutex_lock(&armsmut);
    waiting_arms.erase(arm);
    if (waiting_arms.empty()) {
        release = true;
    }
    pthread_mutex_unlock(&armsmut);
    if (release)
        pthread_cond_broadcast(&armsvar);
}

void
threaded_control_mgr::schedule_fire(scheduler *sched)
{
    pthread_mutex_lock(&condvarmut);
    condvar_triggered = true;
    pthread_mutex_unlock(&condvarmut);
    pthread_cond_broadcast(&condvar);
}

void
threaded_control_mgr::io_fire(scheduler *sched)
{
    if (read(fileno(stdin), &char_to_deliver, 1) < 0);
    sched->remove_io_item(fileno(stdin));

    pthread_mutex_lock(&iovarmut);
    iovar_triggered = true;
    pthread_mutex_unlock(&iovarmut);
    pthread_cond_broadcast(&iovar);
}

/* --------------------------------
   Threaded Services
   --------------------------------*/

class scheduled_lambda : public schedule_item
{
public:
    scheduled_lambda(std::function<void(void)>l) : lambda(l){}
    void schedule_fire(scheduler *) {lambda();delete this;}

private:
    std::function<void(void)>lambda;
};

void
threaded_control_mgr::arm_cycle_forward(scheduler *sched, arm *a)
{
    arm_completion_handler *c = this;
    /* Mark this arm as in use. It is our intent to use it */
    a->use();
    sched->add_schedule_item_us(0,
            new scheduled_lambda(function<void(void)>([a, sched, c](void)
            {
                a->cycle_forward(sched, c);
            })));
}

void
threaded_control_mgr::arm_cycle_backward(scheduler *sched, arm *a)
{
    arm_completion_handler *c = this;
    /* Mark this arm as in use. It is our intent to use it */
    a->use();
    sched->add_schedule_item_us(0,
            new scheduled_lambda(function<void(void)>([a, sched, c](void)
            {
                a->cycle_backward(sched, c);
            })));
}

void
threaded_control_mgr::wait_condvar()
{
    pthread_mutex_lock(&condvarmut);
    if (condvar_triggered) {
        condvar_triggered = false;
    } else {
        pthread_cond_wait(&condvar, &condvarmut);
    }
    pthread_mutex_unlock(&condvarmut);
}

void
threaded_control_mgr::wait_iovar()
{
    pthread_mutex_lock(&iovarmut);
    if (iovar_triggered) {
        iovar_triggered = false;
    } else {
        pthread_cond_wait(&iovar, &iovarmut);
    }
    pthread_mutex_unlock(&iovarmut);
}

void
threaded_control_mgr::wait_arms(arm *arms[], int count)
{
    pthread_mutex_lock(&armsmut);
    for (int i = 0; i < count; i++) {
        if (arms[i]->busy()) {
            waiting_arms.insert(arms[i]);
        }
    }

    if (!waiting_arms.empty()) {
        /* Pay close attention to the order of these operations */
        pthread_cond_wait(&armsvar, &armsmut);
    }
    pthread_mutex_unlock(&armsmut);
}

void
threaded_control_mgr::wait_schedule_item(unsigned us)
{
    condvar_triggered = false;
    sched->add_schedule_item_us(us, this);

    wait_condvar();
}

int
threaded_control_mgr::wait_keypress()
{
    iovar_triggered = false;
    sched->add_io_item(fileno(stdin), this);

    wait_iovar();

    return char_to_deliver;
}

/* -------------------------------
   Thread Control Services
   -------------------------------*/
typedef std::pair<threaded_control_mgr *, threaded_control *> tcpair;

void *
thread_starter(void *arg)
{
    tcpair *p = (tcpair*)arg;
    threaded_control_mgr *tcm = p->first;
    threaded_control *c = p->second;
    delete p;
    c->control(tcm);
}

void
threaded_control_mgr::start_controller(threaded_control *c)
{
    pthread_t t;
    tcpair *p = new tcpair(this, c);
    pthread_create(&t, NULL, thread_starter, (void*)p);
}

void
threaded_control_mgr::controller_done(threaded_control *c)
{
    register_key_handler();
    sched->reloop();
}

