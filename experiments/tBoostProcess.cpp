#include "boost/process.hpp"
#include <iostream>
#include <string>

#include "pugiconfig.hpp"
#include "pugixml.hpp"

namespace bp = boost::process;

int main() {
    bp::ipstream out, error;
    bp::child c(
        "timeout 10 prove -MAthena::Lib --formatter TAP::Formatter::JUnit /home/hdang/p4_dev116/prod/test/perl/Messaging/Broker/t/connect_timeout.t",
        bp::std_out > out, bp::std_err > error);

    std::string outstr, errstr, aLine;
    while (std::getline(out, aLine)) {
        outstr += aLine + "\n";
    }

    aLine.clear();
    while (std::getline(error, aLine)) {
        errstr += aLine + "\n";
    }

    c.wait();

    // std::cout << "STDOUT: " << outstr << "\n\n";
    // std::cout << "STDERR: " << errstr << "\n";
    // std::cout << "exit_code: " << c.exit_code() << "\n";

	// Parse test results using pugixml
	pugi::xml_document doc;
	pugi::xml_parse_result results = doc.load_string(outstr.c_str(), outstr.size());
	if (results) {
		auto node = doc.child("testsuites").child("testsuite");
		std::cout << "Failures: " << node.attribute("failures").value() << "\n";
		std::cout << "Errors: " << node.attribute("errors").value() << "\n";
		std::cout << "Name: " << node.attribute("name").value() << "\n";

		// Get all test cases
		for (pugi::xml_node testcase = node.child("testcase"); testcase; testcase = testcase.next_sibling("testcase")) {
			std::cout << "Test case: " << testcase.attribute("name").value() << "\n";
		}
	} else {
		std::cout << "XML [" << outstr << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
		std::cout << "Error description: " << results.description() << "\n";
		std::cout << "Error offset: " << results.offset << " (error at [..." << (outstr.size() + results.offset) << "]\n\n";
	}
}
