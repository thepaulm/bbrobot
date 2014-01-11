#ifndef _CONFIG_H
#define _CONFIG_H

#include "pwm.h"

struct config_arm
{
    pwm *top;
    unsigned top_min_pwm_us;
    unsigned top_max_pwm_us;
    pwm *bottom;
    unsigned bottom_min_pwm_us;
    unsigned bottom_max_pwm_us;
};

struct config
{
    struct config_arm left_front;
    struct config_arm right_front;
    struct config_arm left_back;
    struct config_arm right_back;
};

config *read_config_file();

#endif /* _CONFIG_H */
