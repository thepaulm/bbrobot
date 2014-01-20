#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <sys/time.h>

#include <vector>
#include <map>
#include <queue>

class schedule_item;
class scheduler;

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

class schedule_item
{
public:
    virtual void schedule_fire(scheduler *) = 0;
};

class io_item
{
public:
    virtual void io_fire(scheduler *) = 0;
};


class reloop_pipe : public io_item
{
public:
    reloop_pipe();
    ~reloop_pipe();
    void reloop();
    void schedule(scheduler *);
    void io_fire(scheduler *);

private:
    int pipe_read;
    int pipe_write;
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
    void add_io_item(int fd, io_item *item);
    io_item *remove_io_item(int fd);          // returns NULL if not found

    /* Signal the loop to start again (to grab new descriptor list) */
    void reloop();

    struct timeval now;
private:
    void update_now();
    int highest_ios;
    std::map<int, io_item *> ios_handlers;
    std::priority_queue<queued_schedule_item,
                        std::vector<queued_schedule_item>,
                        queued_schedule_item_comparitor>timed_handlers;
    unsigned max_ticket; //XXXPAM: Some day this will wrap around
    reloop_pipe rlp;
    bool inselect;
};

extern scheduler *sched;
#endif /* _SCHEDULER_H */
