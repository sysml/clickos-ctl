#ifndef __CLICKOS__H__
#define __CLICKOS__H__

#include <list>
#include <string>


namespace clickos {

/* Planning to support other configuration mechanisms, like vchan */
enum class mechanism {
    xenstore,
};

enum class operation {
    none,
    install,
    remove,
    start,
    stop,
};

} /* namespace clickos */


namespace clickos {
namespace router {

typedef unsigned int id_t;

enum class status_t {
    unknown,
    running,
    stopped,
    error,
};

class router {
public:
    router(id_t id)
        : id(id), status(status_t::unknown), config("")
    {
    }

public:
    const id_t id;
    status_t status;
    std::string config;
};

typedef std::list<router> router_list;

} /* namespace router */
} /* namespace clickos */

#endif /* __CLICKOS__H__ */
