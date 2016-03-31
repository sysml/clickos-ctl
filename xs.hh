/*
 *          ClickOS Control
 *
 *   file: xs.hh
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

#ifndef __XS__H__
#define __XS__H__

#include "clickos.hh"

#include <cerrno>
#include <string>

extern "C" {
#include <xenctrl.h>
#include <xenstore.h>
}


namespace clickos {
namespace xenstore {

/* Define the maximum length of each configuration chunk written to the
 * xenstore. Theoretically this value depends only on the xenstore, but ClickOS
 * is not accepting big chunk sizes, so for now set a value that works.
 *
 * TODO: Check whether chunk_max_len can be increased.
 */
const unsigned int chunk_max_len = 512;
const std::string click_base_path = "/data/clickos";

class xsctl {
public:
    xsctl(void);
    ~xsctl();

public:
    int router_install(const std::string& domain,
            const std::string& name, const std::string& config, router::id_t& rid);
    int router_remove(const std::string& domain, router::id_t rid, bool force);

    int router_set_status(const std::string& domain, router::id_t rid, router::status_t nstatus);

    int router_list(const std::string& domain, std::list<router::id_t>& routers);

private:
    int open(void);
    void close(void);

    int get_and_check_domain(xs_transaction_t xst, const std::string& domain,
            domid_t& domid, std::string& domain_path, std::string& click_path);

    int next_rid(xs_transaction_t xst, const std::string& click_path, router::id_t& rid);

    int name2domid(const std::string& name, domid_t& domid);

    int str2status(const std::string& str, router::status_t& status);
    void status2str(router::status_t status, std::string& str);

private:
    struct xs_handle* xsh;
};

} /* namespace xenstore */
} /* namespace clickos */

#endif /* __XS__H__ */
