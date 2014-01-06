#ifndef _STDIN_HANDLER_H
#define _STDIN_HANDLER_H

#include <termios.h>
#include "scheduler.h"

class stdin_handler : public schedule_item
{
public:
    stdin_handler() :configed(false) {};
    ~stdin_handler() {};

    void fire(scheduler *);
    void config();
    void reset();
private:
    bool configed;
    struct termios orig_termios;
    void run_callibration();
    void inc_servo(int num);
    void dec_servo(int num);
    void commit_callibration();
};

extern stdin_handler key_handler;

#endif /* _STDIN_HANDLER_H */
