#ifndef _PMSSC_H
#define _PMSSC_H

#include <string>
#include <iostream>
#include <fstream>
#include "json/json.h"

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

    bool get_json_config(Json::Value& n);

private:
    bool configure_device();
    bool send_servo_cmd(uint8_t cmd, uint8_t data);
    bool send_servo_cmd(uint8_t cmd, uint8_t data1, uint8_t data2);
    std::string device_path;
    std::ofstream *device;
    uint8_t servo_num;
    bool connected;
    unsigned duty_us;
};

#endif /* _PMSSC_H */
