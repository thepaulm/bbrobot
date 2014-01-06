/* c++ headers */
#include <iostream>

/* c headers */
#include <stdio.h>

/* my c++ headers */
#include "stdin_handler.h"
#include "scheduler.h"
#include "nervous_system.h"

using namespace std;

stdin_handler key_handler;

extern arm *left_arm;
extern arm *right_arm;

void
stdin_handler::config()
{
    struct termios ts;
    tcgetattr(fileno(stdin), &ts);
    orig_termios = ts;
    ts.c_lflag &= ~ICANON;
    ts.c_lflag &= ~ECHO;
    ts.c_cc[VMIN] = 1;
    ts.c_cc[VTIME] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &ts);
    configed = true;
}

void
stdin_handler::reset()
{
    if (configed) {
        tcsetattr(fileno(stdin), TCSANOW, &orig_termios);
        configed = false;
    }
}

#define ARROW_UP 65
#define ARROW_DOWN 66
#define ARROW_RIGHT 67
#define ARROW_LEFT 68

void
stdin_handler::fire(scheduler *psched)
{
    int c = 0;
    if (read(fileno(stdin), &c, 1) > 0) {
        switch (c) {
            case ARROW_RIGHT:
                cout << "right" << endl;
                spine->right_arm->sweep(sched);
                spine->left_leg->sweep(sched);
                break;

            case ARROW_LEFT:
                cout << "left" << endl;
                spine->left_arm->sweep(sched);
                spine->right_leg->sweep(sched);
                break;

            case ARROW_UP:
                cout << "walking ..." << endl;
                spine->walk(sched);
                break;

            case ARROW_DOWN:
                cout << "Stop." << endl;
                spine->stop(sched);
                break;

            default:
                break;
        }
    }
}

