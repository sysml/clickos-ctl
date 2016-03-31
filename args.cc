/*
 *          ClickOS Control
 *
 *   file: args.cc
 *
 *          NEC Europe Ltd. PROPRIETARY INFORMATION
 *
 * This software is supplied under the terms of a license agreement
 * or nondisclosure agreement with NEC Europe Ltd. and may not be
 * copied or disclosed except in accordance with the terms of that
 * agreement. The software and its source code contain valuable trade
 * secrets and confidential information which have to be maintained in
 * confidence.
 * Any unauthorized publication, transfer to third parties or duplication
 * of the object or source code - either totally or in part – is
 * prohibited.
 *
 *      Copyright (c) 2016 NEC Europe Ltd. All Rights Reserved.
 *
 * Authors: Filipe Manco <filipe.manco@neclab.eu>
 *
 * NEC Europe Ltd. DISCLAIMS ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE AND THE WARRANTY AGAINST LATENT
 * DEFECTS, WITH RESPECT TO THE PROGRAM AND THE ACCOMPANYING
 * DOCUMENTATION.
 *
 * No Liability For Consequential Damages IN NO EVENT SHALL NEC Europe
 * Ltd., NEC Corporation OR ANY OF ITS SUBSIDIARIES BE LIABLE FOR ANY
 * DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS
 * OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF INFORMATION, OR
 * OTHER PECUNIARY LOSS AND INDIRECT, CONSEQUENTIAL, INCIDENTAL,
 * ECONOMIC OR PUNITIVE DAMAGES) ARISING OUT OF THE USE OF OR INABILITY
 * TO USE THIS PROGRAM, EVEN IF NEC Europe Ltd. HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 *
 *     THIS HEADER MAY NOT BE EXTRACTED OR MODIFIED IN ANY WAY.
 */

#include "args.hh"
#include "util.hh"

#include <getopt.h>


namespace clickos {

conf::conf(void)
    :
    mech(mechanism::xenstore),
    op(operation::none),

    domain(""),
    router_id(0),

    click_config_path(""),

    install_start(false),
    remove_stop(false),
    remove_force(false),

    help(false)
{
}

bool conf::parse(int argc, char** argv)
{
    const char *short_opts = "+hm:";
    const struct option long_opts[] = {
        { "help"               , no_argument       , NULL , 'h' },
        { "mechanism"          , required_argument , NULL , 'm' },
        { NULL , 0 , NULL , 0 }
    };

    int opt;
    int opt_index;

    bool error = false;

    /* Parse global options */
    while (1) {
        opt = getopt_long(argc, argv, short_opts, long_opts, &opt_index);

        if (opt == -1) {
            break;
        }

        switch (opt) {
            case 'h': {
                help = true;
            } break;

            case 'm': {
                std::string m(optarg);

                if (m == "xenstore") {
                    mech = mechanism::xenstore;
                } else {
                    printf("Invalid mechanism (%s)\n", m.c_str());
                    error = true;
                }
            } break;

            default : {
                error = true;
            } break;
        }
    }

    /* Parse command */
    if (optind < argc) {
        std::string cmd(argv[optind]);

        argc -= optind;
        argv += optind;

        optind = 0;
        optopt = 0;

        if (cmd == "install") {
            op = operation::install;
            error = parse_install(argc, argv);
        } else if (cmd == "remove") {
            op = operation::remove;
            error = parse_remove(argc, argv);
        } else if (cmd == "start") {
            op = operation::start;
            error = parse_start(argc, argv);
        } else if (cmd == "stop") {
            op = operation::stop;
            error = parse_stop(argc, argv);
        } else {
            printf("Invalid command (%s)\n", cmd.c_str());
            error = true;
        }
    } else if (!help) {
        error = true;
    }

    return error;
}


bool conf::parse_install(int argc, char** argv)
{
    const char* short_opts = "s";

    int opt;

    bool error = false;

    while (1) {
        opt = getopt(argc, argv, short_opts);

        if (opt == -1) {
            break;
        }

        switch (opt) {
            case 's': {
                install_start = true;
            } break;

            default : {
                error = true;
            } break;
        }
    }

    if ((argc - optind) != 2) {
        printf("Invalid number of arguments for command install\n");
        error = true;
    } else {
        domain = std::string(argv[optind]);
        click_config_path = std::string(argv[optind + 1]);
    }

    return error;
}

bool conf::parse_remove(int argc, char** argv)
{
    const char* short_opts = "sf";

    int opt;

    bool error = false;

    while (1) {
        opt = getopt(argc, argv, short_opts);

        if (opt == -1) {
            break;
        }

        switch (opt) {
            case 's': {
                remove_stop = true;
            } break;

            case 'f': {
                remove_force = true;
            } break;

            default : {
                error = true;
            } break;
        }
    }

    if ((argc - optind) != 2) {
        printf("Invalid number of arguments for command remove\n");
        error = true;
    } else {
        domain = std::string(argv[optind]);

        if (get_int(argv[optind + 1], router_id)) {
            printf("Invalid router id (%s)\n", argv[optind + 1]);
            error = true;
        }
    }

    return error;
}

bool conf::parse_start(int argc, char** argv)
{
    bool error = false;

    if (argc != 3) {
        printf("Invalid number of arguments for start\n");
        error = true;
    } else {
        domain = std::string(argv[1]);

        if (get_int(argv[2], router_id)) {
            printf("Invalid router id (%s)\n", argv[2]);
            error = true;
        }
    }

    return error;
}

bool conf::parse_stop(int argc, char** argv)
{
    bool error = false;

    if (argc != 3) {
        printf("Invalid number of arguments for stop\n");
        error = true;
    } else {
        domain = std::string(argv[1]);

        if (get_int(argv[2], router_id)) {
            printf("Invalid router id (%s)\n", argv[2]);
            error = true;
        }
    }

    return error;
}

void conf::print_usage(const std::string cmd) {
    printf("Usage: %s [option]... <command> [args...]\n", cmd.c_str());
    printf("\n");
    printf("options:\n");
    printf("  -h, --help                Display this help and exit.\n");
    printf("\n");
    printf("  -m, --mechanism <mech>    Configuration mechanism. Currently only\n");
    printf("                            supports 'xenstore'.\n");
    printf("\n");
    printf("commands:\n");
    printf("    install [option]... <domain> <config_file>\n");
    printf("        Instal a new router in the ClickOS instance.\n");
    printf("\n");
    printf("        -s    Start the configuration immediately after creation.\n");
    printf("\n");
    printf("    remove [option]... <domain> <router_id>\n");
    printf("        Removes a router from the ClickOS instance.\n");
    printf("\n");
    printf("        -s    Stop the router before removal.\n");
    printf("        -f    Force removal of the router, even if in invalid state.\n");
    printf("\n");
    printf("    start <domain> <router_id>\n");
    printf("        Start router execution.\n");
    printf("\n");
    printf("    stop <domain> <router_id>\n");
    printf("        Stop router execution.\n");
}

} /* namespace clickos */
