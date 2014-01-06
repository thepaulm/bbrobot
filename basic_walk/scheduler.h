#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <sys/time.h>

#include <vector>
#include <map>
#include <queue>

class schedule_item;

typedef unsigned schedule_ticket;

class queued_schedule_item
{
public:
    queued_schedule_item(struct timeval _tv,
                         schedule_ticket _id,
                         schedule_item *_item)
        :tv(_tv)
        ,id(_id)
        ,item(_item)
    {}
    struct timeval tv;
    schedule_ticket id;
    schedule_item *item;
};

class queued_schedule_item_comparitor
{
public:
    bool operator()(const queued_schedule_item& lhs,
                    const queued_schedule_item& rhs) const;
};

class scheduler
{
public:
    scheduler();
    ~scheduler();

    void loop();

    /* These are for time based events */
    schedule_ticket add_schedule_item(struct timeval *tv, schedule_item *item);
    schedule_ticket add_schedule_item_us(unsigned us, schedule_item *item);
    schedule_ticket add_schedule_item_ms(unsigned us, schedule_item *item);

    /* These are for io based events */
    void io_item(int fd, schedule_item *item);

    struct timeval now;
private:
    void update_now();
    int highest_ios;
    std::map<int, schedule_item *> ios_handlers;
    std::priority_queue<queued_schedule_item,
                        std::vector<queued_schedule_item>,
                        queued_schedule_item_comparitor>timed_handlers;
    unsigned max_ticket; //XXXPAM: Some day this will wrap around
};

class schedule_item
{
public:
    virtual ~schedule_item() {};
    virtual void fire(scheduler *) = 0;
};

extern scheduler *sched;
#endif /* _SCHEDULER_H */
