#ifndef _CONFIG_H
#define _CONFIG_H

#include "pwm.h"

struct config_arm
{
    pwm *top;
    unsigned high_us;
    unsigned low_us;
    pwm *bottom;
    unsigned forward_us;
    unsigned backward_us;
};

struct config
{
    struct config_arm left_front;
    struct config_arm right_front;
    struct config_arm left_back;
    struct config_arm right_back;
};

config *read_config_file();
bool write_config_file(struct config *);

#endif /* _CONFIG_H */
