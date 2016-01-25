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

const std::string click_base_path = "/data/clickos";

class xsctl {
public:
    xsctl(void);
    ~xsctl();

public:
    int router_install(const std::string& domain, const std::string& config, router::id_t& rid);
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
