#include "args.hh"
#include "clickos.hh"
#include "util.hh"
#include "xs.hh"

#include <list>
#include <string>
#include <cerrno>
#include <cstdint>
#include <cstddef>

extern "C"
{
#include <xenctrl.h>
#include <xenstore.h>
}


static int op_install(clickos::conf& c)
{
    int ret;
    std::string router_config;

    ret = clickos::read_click_config(c.click_config_path, router_config);
    if (ret) {
        return ret;
    }

    ret = 0;
    switch (c.mech) {
        case clickos::mechanism::xenstore: {
            clickos::router::id_t rid;
            clickos::xenstore::xsctl xsc;

            ret = xsc.router_install(c.domain, router_config, rid);
            if (ret) {
                printf("[ %s ] failed to install router.\n", c.domain.c_str());
                break;
            } else {
                printf("[ %s ][ %u ]: installed new router.\n", c.domain.c_str(), rid);
            }

            if (c.install_start) {
                ret = xsc.router_set_status(c.domain, rid, clickos::router::status_t::running);
                if (ret) {
                    printf("[ %s ][ %u ]: failed to start router.\n", c.domain.c_str(), rid);
                    break;
                } else {
                    printf("[ %s ][ %u ]: started router.\n", c.domain.c_str(), rid);
                }
            }
        } break;
    }

    return ret;
}

static int op_remove(clickos::conf& c)
{
    int ret;

    ret = 0;
    switch (c.mech) {
        case clickos::mechanism::xenstore: {
            clickos::xenstore::xsctl xsc;

            if (c.remove_stop) {
                ret = xsc.router_set_status(c.domain, c.router_id,
                        clickos::router::status_t::stopped);
                if (ret) {
                    printf("[ %s ][ %u ]: failed to stop router.\n", c.domain.c_str(), c.router_id);
                    if (!c.remove_force) {
                        break;
                    }
                }
            }

            ret = xsc.router_remove(c.domain, c.router_id, c.remove_force);
            if (ret) {
                printf("[ %s ][ %u ]: failed to remove router.\n", c.domain.c_str(), c.router_id);
            } else {
                printf("[ %s ][ %u ]: router removed.\n", c.domain.c_str(), c.router_id);
            }
        } break;
    }

    return ret;
}

static int op_start(clickos::conf& c)
{
    int ret;

    ret = 0;
    switch (c.mech) {
        case clickos::mechanism::xenstore: {
            clickos::xenstore::xsctl xsc;

            ret = xsc.router_set_status(c.domain, c.router_id, clickos::router::status_t::running);
            if (ret) {
                printf("[ %s ][ %u ]: failed to start router.\n", c.domain.c_str(), c.router_id);
            } else {
                printf("[ %s ][ %u ]: router started.\n", c.domain.c_str(), c.router_id);
            }
        } break;
    }

    return ret;
}

static int op_stop(clickos::conf& c)
{
    int ret;

    ret = 0;
    switch (c.mech) {
        case clickos::mechanism::xenstore: {
            clickos::xenstore::xsctl xsc;

            ret = xsc.router_set_status(c.domain, c.router_id, clickos::router::status_t::stopped);
            if (ret) {
                printf("[ %s ][ %u ]: failed to stop router.\n", c.domain.c_str(), c.router_id);
            } else {
                printf("[ %s ][ %u ]: router stopped.\n", c.domain.c_str(), c.router_id);
            }
        } break;
    }

    return ret;
}


int main (int argc, char** argv)
{
    clickos::conf c;

    if (c.parse(argc, argv)) {
        c.print_usage(argv[0]);
        return -EINVAL;
    }

    if (c.help) {
        c.print_usage(argv[0]);
        return 0;
    }

    switch (c.op) {
        case clickos::operation::install: {
            op_install(c);
        } break;

        case clickos::operation::remove: {
            op_remove(c);
        } break;

        case clickos::operation::start: {
            op_start(c);
        } break;

        case clickos::operation::stop: {
            op_stop(c);
        } break;

        case clickos::operation::none: {
        } break;
    }

    return 0;
}
