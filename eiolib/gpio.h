#ifndef _GPIO_H
#define _GPIO_H

class gpio
{
public:
    gpio(const std::string path);
    virtual ~gpio();

    bool set_value(int value);
    bool on();
    bool off();

private:
    std::string path;
    bool write_file_value(std::string file, int value);
};

gpio *load_gpio(unsigned p, unsigned pin);

#endif /* _GPIO_H */
