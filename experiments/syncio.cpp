#include <boost/process.hpp>
#include <string>

namespace bp = boost::process;

int main() {
    bp::ipstream p;

    bp::child c("ls", bp::std_out > p);

    std::string s;
    std::getline(p, s);

    c.wait();
}
