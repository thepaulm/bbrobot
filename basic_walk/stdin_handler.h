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
};

extern stdin_handler key_handler;

#endif /* _STDIN_HANDLER_H */
