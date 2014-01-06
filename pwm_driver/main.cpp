/* c++ headers */
#include <iostream>

/* c headers */
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

/* my c++ headers */
#include "pwm.h"

using namespace std;

/* globals (mostly for holding reset cleanup state) */
struct termios orig_termios;

#define PWM1 1
#define PWM2 0
#define PWM3 0
#define PWM4 0

pwm *pwm1 = NULL;
pwm *pwm2 = NULL;
pwm *pwm3 = NULL;
pwm *pwm4 = NULL;

void
config_stdin()
{
    struct termios ts;
    tcgetattr(fileno(stdin), &ts);
    orig_termios = ts;
    ts.c_lflag &= ~ICANON;
    ts.c_lflag &= ~ECHO;
    ts.c_cc[VMIN] = 1;
    ts.c_cc[VTIME] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &ts);
}

/* coming from atexit */
void
exiting(void)
{
    cout << "exiting" << endl;
    cout.flush();
    tcsetattr(fileno(stdin), TCSANOW, &orig_termios);
    if (pwm1) {
        pwm1->stop();
        delete pwm1;
        pwm1 = NULL;
    }
    if (pwm2) {
        pwm2->stop();
        delete pwm2;
        pwm2 = NULL;
    }
    if (pwm3) {
        pwm3->stop();
        delete pwm3;
        pwm3 = NULL;
    }
    if (pwm4) {
        pwm4->stop();
        delete pwm4;
        pwm4 = NULL;
    }
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
get_request()
{
    int c = 0;
    if (read(fileno(stdin), &c, 1) >= 0)
        return c;
    return c;
}

int
main(int argc, char *argv[])
{
    int c;
    config_stdin();

    setup_exits();

    int pnum = 0;
    if (argc >= 2)
        pnum = atoi(argv[1]);

#if PWM1
    pwm1 = load_pmssc_pwm("/dev/ttyO4", 3);
#endif
#if PWM2
    //pwm2 = load_native_pwm(9, 16);
    pwm2 = load_pmssc_pwm("/dev/ttyO4", 2);
#endif
#if PWM3
    //pwm3 = load_native_pwm(8, 13);
    pwm3 = load_pmssc_pwm("/dev/ttyO4", 4);
#endif
#if PWM4
    //pwm4 = load_native_pwm(8, 19);
    pwm4 = load_pmssc_pwm("/dev/ttyO4", 6);
#endif

    if (pwm1)
        pwm1->connect();
    if (pwm2)
        pwm2->connect();
    if (pwm3)
        pwm3->connect();
    if (pwm4)
        pwm4->connect();

    while (c = get_request()) {
        pwm *pwmtest = NULL;
        switch (c) {
            case 65:
            case 67:
                if (pwm1) {
                    pwm1->inc();
                    pwmtest = pwm1;
                }
                if (pwm2) {
                    pwm2->inc();
                    pwmtest = pwm2;
                }
                if (pwm3) {
                    pwm3->inc();
                    pwmtest = pwm3;
                }
                if (pwm4) {
                    pwm4->inc();
                    pwmtest = pwm4;
                }
                cout << pwmtest->get_duty_ns() << endl;
                break;

            case 66:
            case 68:
                if (pwm1) {
                    pwm1->dec();
                    pwmtest = pwm1;
                }
                if (pwm2) {
                    pwm2->dec();
                    pwmtest = pwm2;
                }
                if (pwm3) {
                    pwm3->dec();
                    pwmtest = pwm3;
                }
                if (pwm4) {
                    pwm4->dec();
                    pwmtest = pwm4;
                }
                cout << pwmtest->get_duty_ns() << endl;
                break;

            default:
                break;
        }
    } 
    return 0;
}
