#include "util.hh"

#include <fstream>
#include <sstream>


namespace clickos {

int read_click_config(const std::string path, std::string& config)
{
    std::ifstream stream(path);
    std::ostringstream buff;

    buff << stream.rdbuf();

    config = buff.str();

    return 0;
}

} /* namespace clickos */
