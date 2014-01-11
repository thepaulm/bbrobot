/* c++ headers */
#include <string>
#include <iostream>
#include <fstream>

/* c headers */
#include <stdlib.h>

/* my headers */
#include "config.h"
#include "json/json.h"
#include "pwm.h"

#define CONFIG_FILE "basic_walk_config.json"

using namespace std;

void
create_config_servo(pwm **pppwm, unsigned *pmin, unsigned *pmax,
                    Json::Value& val)
{
    if (val["type"] == "pmssc") {
        *pppwm = load_pmssc_pwm(val["path"].asString(), val["number"].asInt());
    } else if (val["type"] == "native") {
        *pppwm = load_native_pwm(val["bank"].asInt(), val["pin"].asInt());
    }
    *pmin = val["min_pwm_us"].asInt();
    *pmax = val["max_pwm_us"].asInt();
}

void
create_config_arm(struct config_arm *cfga, Json::Value arms, const char *name)
{
    Json::Value named = arms[name];
    Json::Value servos = named["servos"];

    create_config_servo(&cfga->top, &cfga->top_min_pwm_us,
                                    &cfga->top_max_pwm_us,
                                    servos["top"]);

    create_config_servo(&cfga->bottom, &cfga->bottom_min_pwm_us,
                                       &cfga->bottom_max_pwm_us,
                                       servos["bottom"]);
}

config *
read_config_file()
{
    const char *homedir = getenv("HOME");
    string filename = homedir + string("/") + string(CONFIG_FILE);
    ifstream ifile; 
    ifile.open(filename.c_str());

    cout << "Reading config file: " << filename << endl;
    Json::Value root;
    Json::Reader jread;

    if (!jread.parse(ifile, root)) {
        cerr << "Failed to read config file!" << endl;
        cerr << jread.getFormatedErrorMessages();
        return NULL;
    }

    ifile.close();

    config *cfg = new config;
    Json::Value arms = root["arms"];

    create_config_arm(&cfg->left_front, arms, "left-front");
    create_config_arm(&cfg->right_front, arms, "right-front");
    create_config_arm(&cfg->left_back, arms, "left-back");
    create_config_arm(&cfg->right_back, arms, "right-back");

    return cfg;
}
