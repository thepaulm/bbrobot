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

using namespace std;

/* ----------------- Configuration of Servos --------------------------- */
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
        if (bank == 0 && pin == 0) {
            type = NONE;
        }
    }
};

struct pwm_config config[] =
{
    {"/dev/ttyO4", 0},              //0
    {"/dev/ttyO4", 1},              //1
    {"/dev/ttyO4", 2},              //2
    {"/dev/ttyO4", 3},              //3
    {8, 19},                        //4
    {9, 16},                        //5
    {9, 14},                        //6
    {8, 13},                        //7
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

pwm *
load_pwm_for_config(struct pwm_config *pconf)
{
    cout << __FUNCTION__ << endl;
    if (pconf->type == SSC) {
        cout << "loading ssc pwm " << pconf->data.ssc.path << ":"
             << pconf->data.ssc.num << endl;
        return load_pmssc_pwm(pconf->data.ssc.path, pconf->data.ssc.num);
    } else if (pconf->type == NATIVE) {
        cout << "loading native pwm " << pconf->data.native.bank << ":"
             << pconf->data.native.pin << endl;
        return load_native_pwm(pconf->data.native.bank, pconf->data.native.pin);
    } else {
        return load_null_pwm();
    }
}

int
main(int argc, char *argv[])
{
    pwm *pwms[8];
    memset(pwms, 0, sizeof(pwm *) * 8);

    /* If we have command line arguments, we assume they are integers
       representing which servos should be turned on. If no command line
       arguments then we run all servos */
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
