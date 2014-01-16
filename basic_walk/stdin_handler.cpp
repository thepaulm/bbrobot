/* c++ headers */
#include <iostream>

/* c headers */
#include <stdio.h>

/* my c++ headers */
#include "stdin_handler.h"
#include "scheduler.h"
#include "nervous_system.h"
#include "config.h"

using namespace std;

stdin_handler key_handler;

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

#define check_exit(c)                                                       \
    if (c == 'c') {                                                         \
        cout << "Exiting config mode." << endl;                             \
        return;                                                             \
    }

void
stdin_handler::inc_servo(int num)
{
    switch (num) {
        case 0:
            spine->left_arm->inc_top();
            break;

        case 1:
            spine->left_arm->inc_bottom();
            break;

        case 2:
            spine->right_arm->inc_top();
            break;

        case 3:
            spine->right_arm->inc_bottom();
            break;

        case 4:
            spine->left_leg->inc_top();
            break;

        case 5:
            spine->left_leg->inc_bottom();
            break;

        case 6:
            spine->right_leg->inc_top();
            break;

        case 7:
            spine->right_leg->inc_bottom();
            break;
    }
}

void
stdin_handler::dec_servo(int num)
{
    switch (num) {
        case 0:
            spine->left_arm->dec_top();
            break;

        case 1:
            spine->left_arm->dec_bottom();
            break;

        case 2:
            spine->right_arm->dec_top();
            break;

        case 3:
            spine->right_arm->dec_bottom();
            break;

        case 4:
            spine->left_leg->dec_top();
            break;

        case 5:
            spine->left_leg->dec_bottom();
            break;

        case 6:
            spine->right_leg->dec_top();
            break;

        case 7:
            spine->right_leg->dec_bottom();
            break;
    }
}

#define each_arm(f)                                                         \
    spine->left_arm->f();                                                   \
    spine->right_arm->f();                                                  \
    spine->left_leg->f();                                                   \
    spine->right_leg->f();

#define each_valve(f)                                                       \
    spine->lfvalve->f();                                                    \
    spine->rfvalve->f();                                                    \
    spine->lbvalve->f();                                                    \
    spine->rbvalve->f();

struct config *
config_from_spine(nervous_system *s)
{
    struct config_arm *c;
    struct config *cfg = new struct config;

    c = spine->left_arm->get_arm_config();
    cfg->left_front = *c;
    delete c;
    c = spine->right_arm->get_arm_config();
    cfg->right_front = *c;
    delete c;
    c = spine->left_leg->get_arm_config();
    cfg->left_back = *c;
    delete c;
    c = spine->right_leg->get_arm_config();
    cfg->right_back = *c;
    delete c; 
    return cfg;
}

void
stdin_handler::run_callibration()
{
    int c = 0;
    int m = 0;
    int servo = 0;

    while (1) {
        while (1) {
            cout << "Enter [f]orward, [b]backward, [u]p, or [d]own ..." << endl;
            if (read(fileno(stdin), &c, 1));
            check_exit(c);
            m = c;
            if (m == 'f' || m == 'b' || m == 'u' || m == 'd')
                break;
            if (c == ' ') {
                cout << "Callibration complete, not saving." << endl;
                return;
            }
            if (c == 's') {
                cout << "Callibration complete, saving ..." << endl;
                write_config_file(config_from_spine(spine));
                cout << "Save complete." << endl;
                return;
            }
        }

        switch (m) {
            case 'f':
                each_arm(request_forward);
                break;
            case 'b':
                each_arm(request_backward);
                break;
            case 'u':
                each_arm(request_up);
                break;
            case 'd':
                each_arm(request_down);
                break;
        }

        cout << "[legs: 0  2]" << endl;
        cout << "[      4  6]" << endl;
        cout << "[arms: 1  3]" << endl;
        cout << "[      5  7]" << endl;

        cout << "Enter servo num or arrow keys to align for " <<
                (char)m << endl;

        while (1) {
            if (read(fileno(stdin), &c, 1));
            check_exit(c);
            if (c == ARROW_RIGHT) {
                inc_servo(servo);
            } else if (c == ARROW_LEFT) {
                dec_servo(servo);
            } else if (c == ' ') {
                switch (m) {
                    case 'f':
                        each_arm(save_forward_state);
                        break;
                    case 'b':
                        each_arm(save_backward_state);
                        break;
                    case 'u':
                        each_arm(save_up_state);
                        break;
                    case 'd':
                        each_arm(save_down_state);
                        break;
                }
                break;
            } else {
                int tmp = c - '0';
                if (tmp >= 0 && tmp <= 7) {
                    servo = tmp;
                    cout << "Servo num " << servo << endl;
                }
            }
        }
    }
}

void
pause_for_key()
{
    int c = 0;
    cout << "Paused ...." << endl;
    if (read(fileno(stdin), &c, 1));
    cout << "Unpause." << endl;
}

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

            case 's':
                cout << "Stand." << endl;
                each_arm(request_standing);
                break;

            case 'h':
                /* Put us in a state right before a walk start */
                cout << "Hang test." << endl;
                spine->right_arm->request_forward();
                spine->right_arm->request_up();
                spine->left_leg->request_forward();
                spine->left_leg->request_up();

                spine->left_arm->request_backward();
                spine->right_leg->request_backward();

                spine->pump_left();
                spine->pump->on();
                break;

            case 'a':
                cout << "Toggling pump." << endl;
                spine->pump->toggle();
                break;

            case 'v':
                cout << "Toggling valves." << endl;
                each_valve(toggle);
                break;

            case 'p':
                pause_for_key();
                break;

            case 'c':
                run_callibration();
            default:
                break;
        }
    }
}

