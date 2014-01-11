/* c++ headers */
#include <string>
#include <iostream>
#include <fstream>

/* c headers */
#include <stdlib.h>
#include <stdio.h>

/* my headers */
#include "config.h"
#include "json/json.h"
#include "pwm.h"

#define CONFIG_FILE "basic_walk_config.json"

using namespace std;

bool
create_config_servo(pwm **pppwm, Json::Value& val)
{
    if (!val.isMember("type")) {
        cerr << "servo has no type: " << val << endl;
        return false;
    }
    if (val["type"] == "pmssc") {
        *pppwm = load_pmssc_pwm(val["path"].asString(), val["number"].asInt());
    } else if (val["type"] == "native") {
        *pppwm = load_native_pwm(val["bank"].asInt(), val["pin"].asInt());
    } else if (val["type"] == "null") {
        *pppwm = load_null_pwm();
    } else {
        cerr << "servo has unknown type: " << val << endl;
        return false;
    }
    return true;
}

bool
create_config_arm(struct config_arm *cfga, Json::Value arms, const char *name)
{
    if (!arms.isMember(name)) {
        cerr << "Failed to find arms." << name << " in config file." << endl;
        return false;
    }
    Json::Value named = arms[name];

    if (!named.isMember("servos")) {
        cerr << "Failed to find arms." << name << ".servos in config file."
             << endl;
        return false;
    }
    Json::Value servos = named["servos"];

    if (!servos.isMember("top")) {
        cerr << "Failed to find arms." << name << ".servos.top in config file."
             << endl;
        return false;
    }
    create_config_servo(&cfga->top, servos["top"]);
    cfga->high_us = servos["top"]["high_us"].asInt();
    cfga->low_us = servos["top"]["low_us"].asInt();

    if (!servos.isMember("bottom")) {
        cerr << "Failed to find arms." << name << ".servos.bottom in config "
                "file." << endl;
        return false;
    }

    create_config_servo(&cfga->bottom, servos["bottom"]);
    cfga->forward_us = servos["bottom"]["forward_us"].asInt();
    cfga->backward_us = servos["bottom"]["backward_us"].asInt();

    return true;;
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
    Json::Value arms;

    if (!jread.parse(ifile, root)) {
        cerr << "Failed to read config file!" << endl;
        cerr << jread.getFormatedErrorMessages();
        return NULL;
    }

    ifile.close();

    config *cfg = new config;
    if (!root.isMember("arms")) {
        cerr << "Found no \"arms\" section in config file." << endl;
        goto fail;
    }
    arms = root["arms"];

    if (!create_config_arm(&cfg->left_front, arms, "left-front"))
        goto fail;
    if (!create_config_arm(&cfg->right_front, arms, "right-front"))
        goto fail;
    if (!create_config_arm(&cfg->left_back, arms, "left-back"))
        goto fail;
    if (!create_config_arm(&cfg->right_back, arms, "right-back"))
        goto fail;

    return cfg;

fail:
    delete cfg;
    return NULL;
}

bool
create_arm_config(struct config_arm *cfga, Json::Value& arms, const char *name)
{
    Json::Value jname = Json::Value(Json::objectValue);
    Json::Value servos = Json::Value(Json::objectValue);

    Json::Value t = Json::Value(Json::objectValue);
    t["high_us"] = Json::Value(cfga->high_us);
    t["low_us"] = Json::Value(cfga->low_us);
    cfga->top->get_json_config(t);

    Json::Value b = Json::Value(Json::objectValue);
    b["forward_us"] = Json::Value(cfga->forward_us);
    b["backward_us"] = Json::Value(cfga->backward_us);
    cfga->bottom->get_json_config(b);

    servos["top"] = t;
    servos["bottom"] = b;

    jname["servos"] = servos;

    arms[name] = jname;

    return true;
}

bool
write_config_file(struct config *cfg)
{
    // write out the tmp file then call rename to slot it in place
    Json::Value root;
    Json::Value n = Json::Value(Json::objectValue);

    create_arm_config(&cfg->left_front, n, "left-front");
    create_arm_config(&cfg->right_front, n, "right-front");
    create_arm_config(&cfg->left_back, n, "left-back");
    create_arm_config(&cfg->right_back, n, "right-back");
    root["arms"] = n;

    const char *homedir = getenv("HOME");
    string filename = homedir + string("/") + string(CONFIG_FILE);
    string tmpfile = homedir + string("/.") + string(CONFIG_FILE) +
                     string(".save");

    ofstream fout;
    fout.open(tmpfile.c_str());
    if (!fout) {
        cerr << "Failed to open " + tmpfile + " for writing." << endl;
        fout.close();
        return false;
    }

    fout << root;
    fout.close();

    rename(tmpfile.c_str(), filename.c_str());

    return true;
}
