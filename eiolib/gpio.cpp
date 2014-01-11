/* c++ headers */
#include <string>
#include <iostream>
#include <fstream>

/* c headers */
#include <string.h>

/* my c++ headers */
#include "gpio.h"
#include "bbdevices.h"

using namespace std;

static std::string *
find_dir_for_gpio(unsigned p, unsigned pin)
{
    int num = 0;

    /* Glob for the path */
    std::string *path = find_ocp_dir_for_glob("gpio-P%d.%d_gpio*", p, pin);
    if (*path == "")
        return path;

    const char *cstr = path->c_str();
    const char *pnumloc = strstr(cstr, "_gpio");

    /* Parse out the number */
    pnumloc += strlen("_gpio");
    while (*pnumloc >= '0' && *pnumloc <= '9') {
        num *= 10;
        num += *pnumloc - '0';
        pnumloc++;
    }
    delete path;

    /* Create the export path */
    path = new string("/sys/class/gpio/gpio");
    char buf[12];
    sprintf(buf, "%d", num);
    *path += buf;
    return path;
}

gpio *
load_gpio(unsigned p, unsigned pin)
{
    gpio *pret = NULL;
    string *path = find_dir_for_gpio(p, pin);
    cout << "found my gpio path: " << *path << endl;
    pret = new gpio(*path);
    delete path;
    return pret;
}

gpio::gpio(const std::string path)
: path(path)
, status(false)
{
}

gpio::~gpio()
{
}

bool
gpio::toggle()
{
    set_value(!status);
    return status;
}

bool
gpio::get_status()
{
    return status;
}

bool
gpio::set_value(int value)
{
    status = (bool)value;
    return write_file_value("value", value);
}

bool
gpio::on()
{
    cout << this << " on" << endl;
    return set_value(1);
}

bool
gpio::off()
{
    cout << this << " off" << endl;
    return set_value(0);
}

bool
gpio::write_file_value(std::string file, int value)
{
    ofstream f;
    string s;
    char buffer[16];

    s = path + "/" + file;
    f.open(s.c_str());
    sprintf(buffer, "%d", value);
    f << buffer;
    f.close();
    return true;
}

