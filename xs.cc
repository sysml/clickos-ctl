/*
 *          ClickOS Control
 *
 *   file: xs.cc
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

#include "xs.hh"
#include "util.hh"

#include <algorithm>
#include <set>


namespace clickos {
namespace xenstore {

xsctl::xsctl(void)
    : xsh(NULL)
{
    /* Opening a xenstore connection may fail and we can't throw exceptions.
     * Therefore initialize xsh in the open method, that needs to be called
     * from all the other methods. */
}

xsctl::~xsctl()
{
    close();
}


int xsctl::router_install(const std::string& domain,
        const std::string& name, const std::string& config, router::id_t& rid)
{
    int ret;
    xs_transaction_t xst;

    ret = open();
    if (ret) {
        return ret;
    }

    ret = 0;
    for ( ; ; ) {
        xst = xs_transaction_start(xsh);
        if (xst == XBT_NULL) {
            ret = errno;
            printf("Failed to start xenstore transaction.\n");
            break;
        }

        domid_t domid;
        std::string domain_path;
        std::string click_path;
        ret = get_and_check_domain(xst, domain, domid, domain_path, click_path);
        if (ret) {
            break;
        }

        /* Determine next free router id */
        router::id_t nrid;
        ret = next_rid(xst, click_path, nrid);
        if (ret) {
            break;
        }

        /* Install configuration on xenstore */
        std::string path;
        std::string router_path = click_path + "/" + std::to_string(nrid);
        std::string entries[] = {"/control", "/elements"};

        for (std::string e : entries) {
            path = router_path + e;
            if (!xs_mkdir(xsh, xst, path.c_str())) {
                ret = errno;
                printf("Failed to write to xenstore.\n");
                break;
            }
        }
        if (ret) {
            break;
        }

        path = router_path + "/config_name";
        if (!xs_write(xsh, xst, path.c_str(), name.c_str(), name.length())) {
            ret = errno;
            printf("Failed to write to xenstore.\n");
            break;
        }

        path = router_path + "/status";
        std::string status_str;
        status2str(router::status_t::stopped, status_str);
        if (!xs_write(xsh, xst, path.c_str(), status_str.c_str(), status_str.length())) {
            ret = errno;
            printf("Failed to write to xenstore.\n");
            break;
        }

        for (unsigned int i = 0 ; ; i++) {
            path = router_path + "/config/" + std::to_string(i);

            unsigned int pos = i * chunk_max_len;
            if (pos >= config.length()) {
                break;
            }

            std::string chunk = config.substr(i * chunk_max_len, chunk_max_len);

            if (!xs_write(xsh, xst, path.c_str(), chunk.c_str(), chunk.length())) {
                ret = errno;
                printf("Failed to write to xenstore.\n");
                break;
            }
        }
        if (ret) {
            break;
        }

        if (!xs_transaction_end(xsh, xst, false)) {
            if (errno == EAGAIN) {
                continue;
            } else {
                ret = errno;
                printf("Failed to write to xenstore.\n");
            }
        } else {
            rid = nrid;
        }

        xst = XBT_NULL;
        break;
    }

    if (xst != XBT_NULL) {
        xs_transaction_end(xsh, xst, true);
        xst = XBT_NULL;
    }

    return ret;
}

int xsctl::router_remove(const std::string& domain, router::id_t rid, bool force)
{
    int ret;
    xs_transaction_t xst;

    ret = open();
    if (ret) {
        return ret;
    }

    ret = 0;
    for ( ; ; ) {
        xst = xs_transaction_start(xsh);
        if (xst == XBT_NULL) {
            ret = errno;
            printf("Failed to start xenstore transaction.\n");
            break;
        }

        domid_t domid;
        std::string domain_path;
        std::string click_path;
        ret = get_and_check_domain(xst, domain, domid, domain_path, click_path);
        if (ret) {
            break;
        }

        /* Determine router status */
        std::string router_path = click_path + "/" + std::to_string(rid);
        std::string status_path = router_path + "/status";

        char* result;
        unsigned int length;
        result = static_cast<char*>(xs_read(xsh, xst, status_path.c_str(), &length));
        if (!result) {
            ret = errno;
            if (ret == ENOENT) {
                printf("Router not found.\n");
            } else {
                printf("Fail to read from xenstore.\n");
            }
            break;
        }

        std::string status_str(result);
        free(result);

        router::status_t cstatus;
        ret = str2status(status_str, cstatus);
        if (ret) {
            printf("Router status is invalid: %s.\n", status_str.c_str());
            break;
        }

        if (cstatus == router::status_t::running) {
            if (force) {
                printf("Router running but force enable, continuing.\n");
            } else {
                ret = EINVAL;
                printf("Cannot remove running router.\n");
                break;
            }
        }

        if (!xs_rm(xsh, xst, router_path.c_str())) {
            ret = errno;
            printf("Failed to write to xenstore.\n");
            break;
        }

        if (!xs_transaction_end(xsh, xst, false)) {
            if (errno == EAGAIN) {
                continue;
            } else {
                ret = errno;
                printf("Failed to write to xenstore.\n");
            }
        }

        xst = XBT_NULL;
        break;
    }

    if (xst != XBT_NULL) {
        xs_transaction_end(xsh, xst, true);
        xst = XBT_NULL;
    }

    return ret;
}

int xsctl::router_set_status(const std::string& domain, router::id_t rid, router::status_t nstatus)
{
    int ret;
    xs_transaction_t xst;

    ret = open();
    if (ret) {
        return ret;
    }

    ret = 0;
    for ( ; ; ) {
        xst = xs_transaction_start(xsh);
        if (xst == XBT_NULL) {
            ret = errno;
            printf("Failed to start xenstore transaction.\n");
            break;
        }

        domid_t domid;
        std::string domain_path;
        std::string click_path;
        ret = get_and_check_domain(xst, domain, domid, domain_path, click_path);
        if (ret) {
            break;
        }

        std::string status_path = click_path + "/" + std::to_string(rid) + "/status";

        char* result;
        unsigned int length;
        result = static_cast<char*>(xs_read(xsh, xst, status_path.c_str(), &length));
        if (!result) {
            ret = errno;
            if (ret == ENOENT) {
                printf("Router not found.\n");
            } else {
                printf("Fail to read from xenstore.\n");
            }
            break;
        }

        std::string status_str(result);
        free(result);

        router::status_t cstatus;
        ret = str2status(status_str, cstatus);
        if (ret) {
            printf("Router status is invalid: %s.\n", status_str.c_str());
            break;
        }

        if (nstatus == cstatus) {
            ret = EINVAL;
            printf("Router already in %s state.\n", status_str.c_str());
            break;
        }

        status2str(nstatus, status_str);

        if (!xs_write(xsh, xst, status_path.c_str(), status_str.c_str(), status_str.length())) {
            ret = errno;
            printf("Failed to write to xenstore.\n");
            break;
        }

        if (!xs_transaction_end(xsh, xst, false)) {
            if (errno == EAGAIN) {
                continue;
            } else {
                ret = errno;
                printf("Failed to write to xenstore.\n");
            }
        }

        xst = XBT_NULL;
        break;
    }

    if (xst != XBT_NULL) {
        xs_transaction_end(xsh, xst, true);
        xst = XBT_NULL;
    }

    return ret;
}


int xsctl::open(void)
{
    if (xsh) {
        return 0;
    }

    xsh = xs_open(0);
    if (xsh == NULL) {
        printf("Failed to connect to xenstore.\n");
        return errno;
    }

    return 0;
}

void xsctl::close(void)
{
    if (!xsh) {
        return;
    }

    xs_close(xsh);
    xsh = NULL;
}

int xsctl::get_and_check_domain(xs_transaction_t xst, const std::string& domain,
        domid_t& domid, std::string& domain_path, std::string& click_path)
{
    int ret;
    char* result;
    unsigned int len;


    ret = name2domid(domain, domid);
    if (ret) {
        if (ret == EINVAL) {
            printf("Invalid domain: %s.\n", domain.c_str());
        }

        return ret;
    }

    result = xs_get_domain_path(xsh, domid);
    if (!result) {
        printf("Failed to retrieve domain path.\n");
        return errno;
    }

    domain_path = std::string(result);
    free(result);

    click_path = domain_path + click_base_path;

    result = static_cast<char*>(xs_read(xsh, xst, click_path.c_str(), &len));
    if (!result) {
        printf("Domain %s, doesn't seem to be a ClickOS domain.\n", domain.c_str());
        return EINVAL;
    }
    free(result);

    return 0;
}

int xsctl::next_rid(xs_transaction_t xst, const std::string& click_path, router::id_t& rid)
{
    char** router_list;
    unsigned int router_num;
    std::set<router::id_t> rids;

    router_list = xs_directory(xsh, XBT_NULL, click_path.c_str(), &router_num);
    if (router_list == NULL) {
        printf("Failed to read from xenstore.\n");
        return EINVAL;
    }

    for (unsigned int i = 0; i < router_num; i++) {
        router::id_t id;

        if (clickos::get_int(router_list[i], id) == 0) {
            rids.insert(id);
        }
    }

    router::id_t nrid = 0;
    while (!rids.empty()) {
        if (nrid != *(rids.begin())) {
            break;
        }

        nrid++;
        rids.erase(rids.begin());
    }

    rid = nrid;

    return 0;
}

int xsctl::name2domid(const std::string& name, domid_t& domid)
{
    int ret;
    char* path;
    char* dom_name;
    char** dom_list;
    unsigned int length;
    unsigned int dom_num;


    if (clickos::get_int(name, domid) == 0) {
        return 0;
    }

    ret = open();
    if (ret) {
        return ret;
    }

    /* All domains should have a directory under /local/domain. Get the list. */
    dom_list = xs_directory(xsh, XBT_NULL, "/local/domain", &dom_num);
    if (dom_list == NULL) {
        printf("Failed to read from xenstore.\n");
        return errno;
    }

    ret = EINVAL;
    for (unsigned int i = 0; i < dom_num; i++) {
        domid_t id;

        /* Check the entry is a number (domid), otherwise it's garbage. Keep the value in case this
         * is the domain we're looking for.
         */
        if (clickos::get_int(dom_list[i], id)) {
            continue;
        }

        /* Get the name under /local/domain/<domid>/name. */
        if (asprintf(&path, "/local/domain/%s/name", dom_list[i]) == -1) {
            ret = ENOMEM;
            printf("Failed to allocate memory.\n");
            break;
        }

        dom_name = static_cast<char*>(xs_read(xsh, XBT_NULL, path, &length));
        free(path);

        /* It could fail to read for multiple reasons, but according to xenstore.h there's no way
         * to know if the entry doesn't exist or it actually failed, so we just ignore it.
         */
        if (dom_name == NULL) {
            continue;
        }

        std::string dom_name_str(dom_name);
        free(dom_name);

        if (name == dom_name_str) {
            ret = 0;
            domid = id;
            break;
        }
    }

    free(dom_list);

    return ret;
}

int xsctl::str2status(const std::string& str, router::status_t& status)
{
    if (str.empty()) {
        status = router::status_t::unknown;
    } else if (str == "Running") {
        status = router::status_t::running;
    } else if (str == "Halted") {
        status = router::status_t::stopped;
    } else if (str == "Error") {
        status = router::status_t::error;
    } else {
        return EINVAL;
    }

    return 0;
}

void xsctl::status2str(router::status_t status, std::string& str)
{
    switch (status) {
        case router::status_t::unknown:
            str = "";
            break;

        case router::status_t::running:
            str = "Running";
            break;

        case router::status_t::stopped:
            str = "Halted";
            break;

        case router::status_t::error:
            str = "Error";
            break;
    }
}

} /* namespace xenstore */
} /* namespace clickos */
