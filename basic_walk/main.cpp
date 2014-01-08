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
#include "scheduler.h"
#include "arm.h"
#include "stdin_handler.h"
#include "nervous_system.h"

struct ssc_config
{
    const char *path;
    unsigned num;
};

struct native_config
{
    unsigned bank;
    unsigned pin;
};

enum pwm_type {NONE, SSC, NATIVE};
struct  pwm_config
{
    enum pwm_type type;
    union {
        struct ssc_config ssc;
        struct native_config native;
    } data;

    pwm_config(const char *path, int num)
    : type(SSC)
    {
        data.ssc.path = path;
        data.ssc.num = num;
    }

    pwm_config(int bank, int pin)
    : type(NATIVE)
    {
        data.native.bank = bank;
        data.native.pin = pin;
        if (bank == pin == 0)
            type = NONE;
    }
};

struct pwm_config config[] =
{
    {"/dev/ttyO4", 0},
    {"/dev/ttyO4", 1},
    {"/dev/ttyO4", 2},
    {"/dev/ttyO4", 3},
    {9, 14},
    {9, 16},
    {8, 19},
    {8, 13},
    {0, 0}
};

using namespace std;

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

pwm *
load_pwm_for_config(struct pwm_config *pconf)
{
    if (pconf->type == SSC)
        return load_pmssc_pwm(pconf->data.ssc.path, pconf->data.ssc.num);
    else if (pconf->type == NATIVE)
        return load_native_pwm(pconf->data.native.bank, pconf->data.native.pin);
    else
        return load_null_pwm();
}

int
main(int argc, char *argv[])
{
    pwm *pwms[8];
    memset(pwms, 0, sizeof(pwm *) * 8);

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            int c = atoi(argv[i]);
            if (c < 8) {
                pwms[c] = load_pwm_for_config(&config[c]);
            } else {
                cout << c << " too high!" << endl;
                return -1;
            }
        }
        for (int i = 0; i < 8; i++) {
            if (!pwms[i])
                pwms[i] = load_null_pwm();
        }
    } else {
        for (int i = 0; i < 8; i++) {
            pwms[i] = load_pwm_for_config(&config[i]);
        }
    }

    key_handler.config();
    setup_exits();

    sched->io_item(fileno(stdin), &key_handler);

    spine = new nervous_system(
                               new arm(pwms[1], pwms[0], LEFT | FRONT),
                               new arm(pwms[3], pwms[2], RIGHT | FRONT),
                               new arm(pwms[5], pwms[4], LEFT | BACK),
                               new arm(pwms[7], pwms[6], RIGHT | BACK)
                               );

    spine->connect();

    sched->loop();

    return 0;
}
