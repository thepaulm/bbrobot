#ifndef _PWM_H
#define _PWM_H

#include <string>
#include "pmssc.h"

class pwm
{
public:

    /* Here is what you need to implement to make one of these */
    pwm(); 
    virtual ~pwm() {};

    virtual bool connect() = 0;
    virtual int set_duty_ns(unsigned) = 0;

    virtual void stop() = 0;
    virtual void start() = 0;

    /* Here is what we provide */

    unsigned get_duty_ns();
    unsigned get_duty_us();

    /* Changing these will change the pct values */
    void set_min_us(unsigned);
    void set_max_us(unsigned);

    /* Values from here are -1 for error, else returns the estimated
       travel time in us */
    int set_duty_pct(unsigned);
    int set_duty_us(unsigned);

    /* Calculate the swing delay from current duty_ns to request. The
       delay is in us, the frequency is ns */
    int get_us_swing_delay_ns(unsigned ns);

    void inc();
    void dec();

protected:
    unsigned duty_ns;
    unsigned step;
    unsigned min_ns;
    unsigned max_ns;
    bool connected;
};

class native_pwm : public pwm
{
public:
    native_pwm(const std::string path);
    ~native_pwm();
    bool connect();
    int set_duty_ns(unsigned);

    void start();
    void stop();

private:
    std::string path;
    bool write_file_value(std::string file, unsigned value);
};

class pmssc_pwm : public pwm
{
public:
    pmssc_pwm(const std::string device_path, unsigned num);
    ~pmssc_pwm();

    bool connect();
    int set_duty_ns(unsigned);
    void start();
    void stop();

    bool ok();

private:
    pmssc *ssc;

};

class null_pwm : public pwm
{
public:
    null_pwm() {}
    ~null_pwm() {}

    bool connect() {return true;}
    int set_duty_ns(unsigned ns); 
    void start() {}
    void stop() {}
    bool ok() {return true;}
};

pwm *load_native_pwm(unsigned p, unsigned pin);
pwm *load_pmssc_pwm(std::string device_path, unsigned num);
pwm *load_null_pwm();

#endif /* _PWM_H */
