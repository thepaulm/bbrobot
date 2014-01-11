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

int
main(int argc, char *argv[])
{
    config *cfg = read_config_file();

    gpio *gpios[5];
    for (int i = 0; i < 5; i++) {
        gpios[i] = load_gpio_for_config(&config_gpio[i]);
    }

    key_handler.config();
    setup_exits();

    sched->io_item(fileno(stdin), &key_handler);

    spine = new nervous_system(
                               /* Left front arm */
                               new arm(cfg->left_front.top,
                                       cfg->left_front.bottom, LEFT | FRONT),
                               gpios[0],

                               /* Right front arm */
                               new arm(cfg->right_front.top,
                                       cfg->right_front.bottom, RIGHT | FRONT),
                               gpios[1],

                               /* Left back leg */
                               new arm(cfg->left_back.top,
                                       cfg->left_back.bottom, LEFT | BACK),
                               gpios[2],

                               /* Right back leg */
                               new arm(cfg->right_back.top,
                                       cfg->right_back.bottom, RIGHT | BACK),
                               gpios[3],

                               /* Vacuum pump */
                               gpios[4]
                               );

    delete cfg;

    spine->connect();

    sched->loop();

    return 0;
}
