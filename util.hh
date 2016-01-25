#ifndef __UTIL__HH__
#define __UTIL__HH__

#include <limits>
#include <string>
#include <stdexcept>


namespace clickos {

template<typename int_t>
int get_int(const std::string arg, int_t& number)
{
    long long n;

    try {
        n = std::stoll(arg.c_str());
    } catch(std::invalid_argument e) {
        return EINVAL;
    } catch(std::out_of_range e) {
        return EINVAL;
    }

    if (n < std::numeric_limits<int_t>::min() || n > std::numeric_limits<int_t>::max()) {
        return EINVAL;
    }

    number = static_cast<int_t>(n);

    return 0;
}

int read_click_config(const std::string path, std::string& config);

} /* namespace clickos */

#endif /*  __UTIL__HH__ */
