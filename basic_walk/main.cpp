/* c++ headers */
#include <iostream>

/* c headers */
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

/* my c++ headers */
#include "pwm.h"
#include "gpio.h"
#include "scheduler.h"
#include "arm.h"
#include "stdin_handler.h"
#include "nervous_system.h"
#include "config.h"

using namespace std;

/* ----------------- Configuration of Solenoids ------------------------- */
struct gpio_config
{
    unsigned p;
    unsigned pin;
};

struct gpio_config config_gpio[] =
{
    {9, 12},                        //0
    {9, 15},                        //1
    {9, 23},                        //2
    {9, 25},                        //3
    {9, 27},                        //4
    {0, 0}
};

/* ---------------------- Unix exit handlers -----------------------------*/
/* coming from atexit */
void
exiting(void)
{
    cout << "exiting" << endl;
    /* reset the terminal */
    key_handler.reset();

    /* cleanup the pwms */
    spine->disconnect();
}

/* coming from signal */
void
sigexit(int)
{
    exit(0);
}

void
setup_exits()
{
    atexit(exiting);
    signal(SIGTERM, sigexit);
    signal(SIGINT, sigexit);
}

gpio *
load_gpio_for_config(struct gpio_config *pconf)
{
    return load_gpio(pconf->p, pconf->pin);
}

arm *
load_arm_from_config(struct config_arm *cfga, int flags)
{
   return new arm(cfga->top,
                  cfga->bottom,
                  cfga->high_us,
                  cfga->low_us,
                  cfga->forward_us,
                  cfga->backward_us,
                  LEFT | FRONT);
}

int
main(int argc, char *argv[])
{
    config *cfg = read_config_file();
    if (!cfg)
        return -1;

    gpio *gpios[5];
    for (int i = 0; i < 5; i++) {
        gpios[i] = load_gpio_for_config(&config_gpio[i]);
    }

    key_handler.config();
    setup_exits();

    register_key_handler();

    spine = new nervous_system(
                               /* Left front arm */
                               load_arm_from_config(&cfg->left_front,
                                                   LEFT | FRONT), gpios[0],

                               /* Right front arm */
                               load_arm_from_config(&cfg->right_front,
                                                    RIGHT | FRONT), gpios[1],

                               /* Left back leg */
                               load_arm_from_config(&cfg->left_back,
                                                    LEFT | BACK), gpios[2],

                               /* Right back leg */
                               load_arm_from_config(&cfg->right_back,
                                                    RIGHT | BACK), gpios[3],

                               /* Vacuum pump */
                               gpios[4]
                               );

    delete cfg;

    spine->connect();

    sched->loop();

    return 0;
}
