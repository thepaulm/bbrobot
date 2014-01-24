#include <iostream>
#include <stdlib.h>
#include <sys/select.h>
#include "scheduler.h"
#include "timevals.h"

scheduler *sched = new scheduler();

using namespace std;

reloop_pipe::reloop_pipe()
{
    int fds[2];
    pipe(fds);
    pipe_read = fds[0];
    pipe_write = fds[1];
}

reloop_pipe::~reloop_pipe()
{
    close(pipe_read);
    close(pipe_write);
}

void
reloop_pipe::reloop()
{
    int c = 0;
    write(pipe_write, &c, 1);
}

void
reloop_pipe::schedule(scheduler *sched)
{
    sched->add_io_item(pipe_read, this);
}

void
reloop_pipe::io_fire(scheduler *sched)
{
    int c;
    read(pipe_read, &c, 1);
}

bool
queued_schedule_item_comparitor::operator()
    (const queued_schedule_item& lhs,
     const queued_schedule_item& rhs) const
{
    if (tv_compare(lhs.tv, rhs.tv) <= 0)
        return false;
    return true;
}

scheduler::scheduler()
: highest_ios(0)
, max_ticket(0)
, inselect(false)
{
    pthread_mutex_init(&elements_mutex, NULL);
    rlp.schedule(this);
}

scheduler::~scheduler()
{
    pthread_mutex_destroy(&elements_mutex);
}

void
scheduler::reloop()
{
    rlp.reloop();
}

void
scheduler::update_now()
{
    gettimeofday(&now, NULL);
}

schedule_ticket
scheduler::add_schedule_item(struct timeval *tv, schedule_item *item)
{
    schedule_ticket t = max_ticket++;
    queued_schedule_item qsi(*tv, t, item);
    pthread_mutex_lock(&elements_mutex);
    timed_handlers.push(qsi);
    pthread_mutex_unlock(&elements_mutex);
    if (inselect)
        reloop();
    return t;
}

schedule_ticket
scheduler::add_schedule_item_us(unsigned us, schedule_item *item)
{
    struct timeval tv = now;
    tv_plus_us(&tv, us);
    return add_schedule_item(&tv, item);
}

schedule_ticket
scheduler::add_schedule_item_ms(unsigned ms, schedule_item *item)
{
    struct timeval tv = now;
    tv_plus_ms(&tv, ms);
    return add_schedule_item(&tv, item);
}

void
scheduler::loop()
{
    int c;
    fd_set set;

    pthread_mutex_lock(&elements_mutex);
    while (1) {

        update_now();

        /* Find the next scheduled time element */
        struct timeval *ptv = NULL;
        struct timeval tv;

        /* This is the barrier that says we have already built the select
           list. If we are past this point then any select list modifying
           methods must reloop */
        inselect = true;
        if (timed_handlers.size()) {
            tv = timed_handlers.top().tv;
            tv_minus_floor(&tv, &now);
            ptv = &tv;
        }

        /* Handle the io elements */
        FD_ZERO(&set);
        for (auto it = ios_handlers.begin(); it != ios_handlers.end(); ++it) {
            FD_SET(it->first, &set);
        }

        pthread_mutex_unlock(&elements_mutex);
        int got = select(highest_ios + 1, &set, NULL, NULL, ptv);
        pthread_mutex_lock(&elements_mutex);

        /* We have exited the select. Anybody needing the modify the select
           list elements may do so */
        inselect = false;

        if (got > 0) {
            /* XXX Need to check this for recursion - can we screw this
               iteration up by modifying it? I bet we can */
            ios_handlers_running = ios_handlers;
            for (auto it = ios_handlers_running.begin();
                      it != ios_handlers_running.end();
                      ++it) {
                if (FD_ISSET(it->first, &set)) {
                    pthread_mutex_unlock(&elements_mutex);
                    it->second->io_fire(this);
                    pthread_mutex_lock(&elements_mutex);
                }
            }
            ios_handlers_running.clear();
        }

        update_now();
        while (timed_handlers.size()) {
            tv = timed_handlers.top().tv;
            if (tv_compare(tv, now) <= 0) {
                schedule_item *item = timed_handlers.top().item;
                timed_handlers.pop();
                pthread_mutex_unlock(&elements_mutex);
                item->schedule_fire(sched);
                pthread_mutex_lock(&elements_mutex);
            } else {
                break;
            }
        }
    }
    pthread_mutex_unlock(&elements_mutex);
}

void
scheduler::add_io_item(int fd, io_item *item)
{
    pthread_mutex_lock(&elements_mutex);
    if (fd > highest_ios) {
        highest_ios = fd;
    }
    /* XXXPAM: Check for duplicate before adding */
    ios_handlers.insert(std::pair<int, io_item *>(fd, item)); 

    pthread_mutex_unlock(&elements_mutex);

    if (inselect)
        reloop();
}

io_item *
scheduler::remove_io_item(int fd)
{
    io_item *ret = NULL;
    pthread_mutex_lock(&elements_mutex);
    if (ios_handlers.count(fd) > 0) {
        ret = ios_handlers.at(fd);
        ios_handlers.erase(fd);
    }
    pthread_mutex_unlock(&elements_mutex);
    return ret;
}
