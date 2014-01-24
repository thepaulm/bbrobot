/* c++ headers */
#include <string>
#include <iostream>
#include <fstream>

/* c headers */
#include <glob.h>
#include <stdlib.h>

/* my c++ headers */
#include "pwm.h"
#include "bbdevices.h"

#define NS_PER_MS (1000 * 1000)
#define NS_PER_US (1000)
#define PERIOD_NS (20 * NS_PER_MS)

#define DEFAULT_DUTY_NS (1500 * NS_PER_US)

#define EXPECTED_RANGE_NS (1 * NS_PER_MS)
#define STEPS 100

#define STEP_SIZE (EXPECTED_RANGE_NS / STEPS)

#define DEFAULT_MIN_NS (1 * 1000 * 1000)
#define DEFAULT_MAX_NS (2 * 1000 * 1000)

#define DELAY_PER_PWM_US__US (0.8)

using namespace std;

/* class creation method */
static std::string *
find_dir_for_pwm(unsigned p, unsigned pin)
{
    return find_ocp_dir_for_glob("pwm_test_P%d_%d.*", p, pin);
}

pwm *
load_native_pwm(unsigned p, unsigned pin)
{
    cout << "load_native_pwm: " << p << " : " << pin << endl;
    pwm *pret = NULL;
    string *path;
    path = find_dir_for_pwm(p, pin);
    pret = new native_pwm(*path, p, pin);
    delete path;
    return pret;
}

pwm *
load_pmssc_pwm(const std::string device_path, unsigned servo_num)
{
    cout << "load_pmssc_pwm: " << device_path << " : " << servo_num << endl;
    pmssc_pwm *p = new pmssc_pwm(device_path, servo_num);
    if (!p->ok()) {
        delete p;
        return NULL;
    }
    return p;
}

pwm *
load_null_pwm()
{
    return (pwm *)new null_pwm();
}

/****************************************************************************

    generic pwm methods

 ****************************************************************************/
pwm::pwm()
: duty_ns(DEFAULT_DUTY_NS)
, step(STEP_SIZE)
, min_ns(DEFAULT_MIN_NS)
, max_ns(DEFAULT_MAX_NS)
, connected(false)
{
}

/* get / set */
unsigned
pwm::get_duty_ns()
{
    return duty_ns;
}

unsigned
pwm::get_duty_us()
{
    return duty_ns / NS_PER_US;
}

int
pwm::set_duty_us(unsigned d)
{
    return set_duty_ns(d * NS_PER_US);
}

int
pwm::set_duty_pct(unsigned p)
{
    return set_duty_ns(min_ns + ((float)p / 100.0) * (max_ns - min_ns));
}

unsigned
pwm::get_duty_pct()
{
    return 100.0 * (float)(duty_ns - min_ns) / (float)(max_ns - min_ns);
}

/* These will change what the pct is measured relative to */
void
pwm::set_min_us(unsigned m)
{
    min_ns = m * NS_PER_US;
}

void
pwm::set_max_us(unsigned m)
{
    max_ns = m * NS_PER_US;
}

/* Increment / decrement by "step", where step is range / STEP_SIZE */
void
pwm::inc()
{
    set_duty_ns(duty_ns + step);
}

void
pwm::dec()
{
    set_duty_ns(duty_ns - step);
}

int
pwm::get_us_swing_delay_ns(unsigned ns)
{
    unsigned d_us;
    if (duty_ns > ns) {
        d_us = (duty_ns - ns) / NS_PER_US;
    } else {
        d_us = (ns - duty_ns) / NS_PER_US;
    }

    return d_us * DELAY_PER_PWM_US__US;
}

/****************************************************************************

    native_pwm methods

 ****************************************************************************/

/* initialization */
native_pwm::native_pwm(const std::string path, unsigned bank, unsigned pin)
: path(path)
, bank(bank)
, pin(pin)
{
}

native_pwm::~native_pwm()
{
}

/* main worker for almost everything */

#define DO_FILE_WRITE 1
bool
native_pwm::write_file_value(std::string file, unsigned value)
{
    if (!connected)
        return false;
#if DO_FILE_WRITE
    ofstream f;
    string s;
    char buffer[16];

    s = path + "/" + file;
    f.open(s.c_str());
    sprintf(buffer, "%u", value);
    f << buffer;
    f.close();
#endif /* DO_FILE_WRITE */
    return true;
}

bool
native_pwm::connect()
{
    /* need to do this here to get the file writes to work */
    connected = true;

    /* no signal while we config */
    stop();

    /* set polarity */
    write_file_value("polarity", 0);

    /* set the period */
    write_file_value("period", PERIOD_NS);

    /* set the default duty */
    set_duty_ns(duty_ns);

    /* start the signal */
    start();

    return true;
}

/* These next three all return -1 for error, or the estimated transit
   time in us */
int
native_pwm::set_duty_ns(unsigned d)
{
    int delay = get_us_swing_delay_ns(d);

    cout << "(native) new duty: " << d << endl;
    if (write_file_value("duty", d)) {
        duty_ns = d;
        if (delay < 0) {
            cout << "native_pwm::set_duty_ns returning " << delay << endl;
        }
        return delay;
    } else {
        duty_ns = d;
        cout << "native_pwm::set_duty_ns returning -1" << endl;
        return -1;
    }
}

/* stop and start */
void
native_pwm::stop()
{
    write_file_value("run", 0);
}

void
native_pwm::start()
{
    write_file_value("run", 1);
}

bool
native_pwm::get_json_config(Json::Value& n)
{
    n["type"] = Json::Value("native");
    n["bank"] = Json::Value(bank);
    n["pin"] = Json::Value(pin);
    return true;
}

/****************************************************************************

    pmssc_pwm methods

 ****************************************************************************/
pmssc_pwm::pmssc_pwm(const std::string device_path, unsigned servo_num)
{
    ssc = new pmssc(device_path, servo_num);
}

pmssc_pwm::~pmssc_pwm()
{
    delete ssc;
    ssc = NULL;
}

bool
pmssc_pwm::connect()
{
    if (ssc->connect())
        connected = true;
    return connected;
}

int
pmssc_pwm::set_duty_ns(unsigned ns)
{
    int delay = get_us_swing_delay_ns(ns);

    cout << "(pmssc) new duty: " << ns << endl;
    if (ssc->set_duty_ns(ns)) {
        duty_ns = ns;
        return delay;
    } else {
        duty_ns = ns;
        cout << "pmssc_pwm::set_duty_ns returning -1" << endl;
        return 0;
    }
}

void
pmssc_pwm::stop()
{
    ssc->stop();
}

void
pmssc_pwm::start()
{
    ssc->start();
}

bool
pmssc_pwm::ok()
{
    if (!ssc)
        return false;
    return ssc->ok();
}

bool
pmssc_pwm::get_json_config(Json::Value& n)
{
    return ssc->get_json_config(n);
}

/***************************************************************************

  null_pwm methods

 ****************************************************************************/
int
null_pwm::set_duty_ns(unsigned ns)
{
    int delay = get_us_swing_delay_ns(ns);
    duty_ns = ns;
    return delay;
}

bool
null_pwm::get_json_config(Json::Value& n)
{
    n["type"] = Json::Value("null");
    return true;
}

