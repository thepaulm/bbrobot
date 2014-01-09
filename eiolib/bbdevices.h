#ifndef _BB_DEVICES
#define _BB_DEVICES

#define OCP_PATH "/sys/devices/ocp.2"

std::string * find_ocp_dir_for_glob(const std::string glob,
                                    unsigned p, unsigned pin);

#endif /* _BB_DEVICES */

