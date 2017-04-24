#include <iostream>
#include <sstream>

#include "boost/process.hpp"
#include "boost/filesystem.hpp"


namespace bp = boost::process;

#include "fmt/format.h"

int main()
{
	bp::ipstream pipe_stream;
	bp::child c("gcc --version", bp::std_out > pipe_stream);

    std::string line;

    while (pipe_stream && std::getline(pipe_stream, line) && !line.empty()){
		fmt::print("{}\n", line);
	}
 
    c.wait();

	// std::stringstream ss;
	// ss << pipe_stream.rdbuf();
	// fmt::print("{}", ss.str());

	using path = boost::filesystem::path;

	path p("/home/");
	bp::system(p, "ls");
}
