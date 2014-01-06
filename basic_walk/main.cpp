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

int
main(int argc, char *argv[])
{
    pwm *pwms[8];
    memset(pwms, 0, sizeof(pwm *) * 8);

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            int c = atoi(argv[i]);
            if (c < 8) {
                pwms[c] = load_pmssc_pwm("/dev/ttyO4", c);
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
            pwms[i] = load_pmssc_pwm("/dev/ttyO4", i);
        }
    }

    /*
    pwms[0] = load_native_pwm(8, 13);
    pwms[1] = load_native_pwm(8, 19);
    pwms[2] = load_native_pwm(9, 14);
    pwms[3] = load_native_pwm(9, 16);

    pwms[4] = load_null_pwm();
    pwms[5] = load_null_pwm();
    pwms[6] = load_null_pwm();
    pwms[7] = load_null_pwm();
    */

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
