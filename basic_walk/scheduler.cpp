#include <iostream>
#include <stdlib.h>
#include <sys/select.h>
#include "scheduler.h"
#include "timevals.h"

scheduler *sched = new scheduler();

using namespace std;

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
{
}

scheduler::~scheduler()
{
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
    timed_handlers.push(qsi);
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
    fd_set set;
    while (1) {

        update_now();

        /* Find the next scheduled time element */
        struct timeval *ptv = NULL;
        struct timeval tv;
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

        int got = select(highest_ios + 1, &set, NULL, NULL, ptv);
        if (got > 0) {
            for (auto it = ios_handlers.begin();it != ios_handlers.end();++it) {
                if (FD_ISSET(it->first, &set)) {
                    it->second->fire(this);
                }
            }
        }

        update_now();
        while (timed_handlers.size()) {
            tv = timed_handlers.top().tv;
            if (tv_compare(tv, now) <= 0) {
                schedule_item *item = timed_handlers.top().item;
                item->fire(sched);
                timed_handlers.pop();
            } else {
                break;
            }
        }
        
    }
}

void
scheduler::io_item(int fd, schedule_item *item)
{
    if (fd > highest_ios) {
        highest_ios = fd;
    }
    /* XXXPAM: Check for duplicate before adding */
    ios_handlers.insert(std::pair<int, schedule_item *>(fd, item)); 
}
