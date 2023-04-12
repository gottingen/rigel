#include <rigel/util/std-compat.hh>
#include <rigel/core/sstring.hh>
#include <boost/lexical_cast.hpp>

namespace seastar {

sstring read_first_line(std::filesystem::path sys_file);

template <typename Type>
Type read_first_line_as(std::filesystem::path sys_file) {
    return boost::lexical_cast<Type>(read_first_line(sys_file));
}

}
