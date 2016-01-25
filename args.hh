#ifndef __ARGS__HH__
#define __ARGS__HH__

#include "clickos.hh"

#include <list>
#include <string>

#include <cerrno>
#include <cstdint>

extern "C" {
#include <xenctrl.h>
}


namespace clickos {

class conf {
public:
    conf(void);

public:
    bool parse(int argc, char** argv);
    void print_usage(const std::string cmd);

private:
    bool parse_install(int argc, char** argv);
    bool parse_remove(int argc, char** argv);
    bool parse_start(int argc, char** argv);
    bool parse_stop(int argc, char** argv);

public:
    mechanism mech;
    operation op;

    std::string domain;
    clickos::router::id_t router_id;
    std::string click_config_path;

    bool install_start;
    bool remove_stop;
    bool remove_force;

    bool help;
};

} /* namespace clickos */

#endif /*  __ARGS__HH__ */
