#include "pmssc.h"
#include<ext/stdio_filebuf.h>
#include <termios.h>
#include <unistd.h>
#include <string>
#include <unordered_map>

using namespace std;

#define NS_PER_US 1000

enum CMD_TYPE {SET_PARAM=0,
               SET_SPEED=1,
               SET_POSITION7=2,
               SET_POSITION8=3,
               SET_POSITION_ABS=4,
               SET_NEUTRAL=5};

pmssc::pmssc(const std::string device_path, uint8_t servo_num)
: device_path(device_path)
, servo_num(servo_num)
, device(NULL)
, connected(false)
{
}

pmssc::~pmssc()
{
}

/* These are all so that we can get the descriptor from our ofstream.
   Taken from here:
http://stackoverflow.com/questions/109449/getting-a-file-from-a-stdfstream
*/

typedef std::basic_ofstream<char>::__filebuf_type buffer_t;
typedef __gnu_cxx::stdio_filebuf<char> io_buffer_t; 

FILE *
cfile_impl(buffer_t* const fb)
{
    return (static_cast<io_buffer_t* const>(fb))->file(); //type std::__c_file
}

bool
pmssc::configure_device()
{
    FILE *unixf = cfile_impl(device->rdbuf());
    if (!unixf)
        return false;
    int fd = fileno(unixf);
    
    struct termios ts;
    if (tcgetattr(fd, &ts) < 0)
        return false;
    cfmakeraw(&ts);
    cfsetospeed(&ts, B9600);
    cfsetispeed(&ts, B9600);
    ts.c_cc[VMIN] = 1;
    ts.c_cc[VTIME] = 0;
    if (tcsetattr(fd, TCSANOW, &ts) < 0)
        return false;

    return true;
}

/* This is the global cache of open ofstreams. */
unordered_map<string, ofstream*>open_devices;

bool
pmssc::connect()
{
    if (open_devices.count(device_path) > 0) {
        cout << "We already have " << device_path << "open." << endl;
        device = open_devices[device_path];
    } else {
        device = new ofstream();
        device->open(device_path.c_str(), ofstream::out | ofstream::app);
        if (!device->is_open()) {
            cerr << "Failed to open device " << device_path << endl;
            return false;
        }
        cout << "I opened and cached " << device_path << endl;
        open_devices[device_path] = device;
    }

    if (!configure_device()) {
        cerr << "Could not configure device " << device_path << endl;
        return false;
    }

    connected = true;
    return start();
}

bool
pmssc::ok()
{
    /* XXXPAM: Someday this should check something */
    return true;
}

bool
pmssc::send_servo_cmd(uint8_t cmd, uint8_t d)
{
    if (!connected)
        return false;
    uint8_t data[5];
    data[0] = 0x80;
    data[1] = 0x01;
    data[2] = cmd;
    data[3] = servo_num;
    data[4] = d;

    device->write((const char *)data, 5);
    device->flush();
    return (*device);
}

bool
pmssc::send_servo_cmd(uint8_t cmd, uint8_t d1, uint8_t d2)
{
    if (!connected)
        return false;
    uint8_t data[6];
    data[0] = 0x80;
    data[1] = 0x01;
    data[2] = cmd;
    data[3] = servo_num;
    data[4] = d1;
    data[5] = d2;

    device->write((const char*)data, 6);
    device->flush();
    return (*device);
}

bool
pmssc::set_duty_ns(unsigned ns)
{
    return set_duty_us(ns / NS_PER_US);
}

bool
pmssc::set_duty_us(unsigned us)
{
    unsigned pmssc_tics = us * 2;
    uint8_t data1, data2;
    data1 = pmssc_tics >> 7;
    data2 = pmssc_tics & ((1 << 7) - 1);
    return send_servo_cmd(SET_POSITION_ABS, data1, data2);
}


/* PARAM BITS:

   bit 7: always 0
   bit 6: servo on/off
   bit 5: direction (for 7/8 bit position commands)
   bit 0-4: range (for 7/8 bit position commands)    
 */

bool
pmssc::stop()
{
    cout << "sending set param 0" << endl;
    return send_servo_cmd(SET_PARAM, 0);
}

bool
pmssc::start()
{
    cout << "send set param 1 << 6" << endl;
    return send_servo_cmd(SET_PARAM, 1 << 6);
}

bool
pmssc::get_json_config(Json::Value& n)
{
    n["type"] = Json::Value("pmssc");
    n["path"] = Json::Value(device_path);
    n["number"] = Json::Value(servo_num);
    return true;
}
