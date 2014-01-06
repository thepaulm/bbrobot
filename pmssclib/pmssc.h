#ifndef _PMSSC_H
#define _PMSSC_H

#include <string>
#include <iostream>
#include <fstream>

typedef unsigned char uint8_t;

/* This class controls the Pololu Micro Serial Servo Controller board. */
class pmssc {
public:
    pmssc(const std::string device_path, uint8_t servo_num);
    ~pmssc();

    /* Connect to the device. This will also initiate a start() */
    bool connect();

    bool set_duty_ns(unsigned);
    bool set_duty_us(unsigned); 

    bool stop();
    bool start();
    bool ok();

private:
    bool configure_device();
    bool send_servo_cmd(uint8_t cmd, uint8_t data);
    bool send_servo_cmd(uint8_t cmd, uint8_t data1, uint8_t data2);
    std::string device_path;
    std::ofstream *device;
    uint8_t servo_num;

};

#endif /* _PMSSC_H */