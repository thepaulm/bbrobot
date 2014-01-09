#include <string>
#include <iostream>
#include <fstream>

#include <glob.h>
#include <stdlib.h>

#include "bbdevices.h"

std::string *
find_ocp_dir_for_glob(const std::string globstr, unsigned p, unsigned pin)
{
    std::string *ret = NULL;
    glob_t g;
    char glob_buffer[64];
    int at = sprintf(glob_buffer, "%s/", OCP_PATH);
    sprintf(glob_buffer + at, globstr.c_str(), p, pin);

    g.gl_offs = 1;
    glob(glob_buffer, 0, NULL, &g);
    if (g.gl_pathc) {
        ret = new std::string(g.gl_pathv[0]);
    } else {
        ret = new std::string("");
    }
    globfree(&g);

    return ret;
}
