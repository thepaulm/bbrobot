#include <pthread.h>
#include "threaded_control.h"
#include "scheduler.h"
#include "stdin_handler.h"

using namespace std;

threaded_control_mgr::threaded_control_mgr()
{
    cout << "INITIALIZING the threaded_control_mgr" << endl;
    if (0 != pthread_cond_init(&condvar, NULL)) {
        cerr << "pthread_cond_init failed." << endl;
    }
    if (0 != pthread_mutex_init(&condvarmut, NULL)) {
        cerr << "pthread_mutex_init failed." << endl;
    }
}

threaded_control_mgr::~threaded_control_mgr()
{
    pthread_cond_destroy(&condvar);
    pthread_mutex_destroy(&condvarmut);
}

/* --------------------------------
   System Upcalls
   --------------------------------*/
void
threaded_control_mgr::finish(scheduler *sched, arm *arm)
{
}

void
threaded_control_mgr::schedule_fire(scheduler *sched)
{
    pthread_cond_broadcast(&condvar);
}

void
threaded_control_mgr::io_fire(scheduler *sched)
{
    read(fileno(stdin), &char_to_deliver, 1);
    sched->remove_io_item(fileno(stdin));
    pthread_cond_broadcast(&condvar);
}

/* --------------------------------
   Threaded Services
   --------------------------------*/

void
threaded_control_mgr::wait_condvar()
{
    pthread_mutex_lock(&condvarmut);
    pthread_cond_wait(&condvar, &condvarmut);
    pthread_mutex_unlock(&condvarmut);
}

    /* XXX This isn't threadsafe! Move to a request queue model to 
       go from this control thread to the driver thread */

void
threaded_control_mgr::wait_arms(arm *arms[], int count)
{
}

void
threaded_control_mgr::wait_schedule_item(unsigned us)
{
    sched->add_schedule_item_us(us, this);

    wait_condvar();
}

int
threaded_control_mgr::wait_keypress()
{
    sched->add_io_item(fileno(stdin), this);

    wait_condvar();

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
    cout << "threaded_control_mgr::controller_done" << endl;
    register_key_handler();
    sched->reloop();
}

