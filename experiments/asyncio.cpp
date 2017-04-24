#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <string>
#include <iostream>


namespace bp = boost::process;

int main()
{
    boost::asio::io_service ios;
    boost::asio::streambuf buffer;


    bp::child c(
        "ls",
        bp::std_out > buffer,
        ios
        );

    ios.run();
}
