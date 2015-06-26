#ifndef InputArgumentParser_hpp
#define InputArgumentParser_hpp

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include "boost/lexical_cast.hpp"
#include <string>
#include <vector>
#include <iostream>

namespace Tools {
  struct InputArgumentParser {
  public:
    InputArgumentParser(int argc, char *argv[]) {
        using namespace boost;
        namespace po = boost::program_options;
        po::options_description desc("Allowed options");
        desc.add_options()              
            ("help,h", "Print this help")
            ("verbose,v", "Display searched data.")
            ("folder,f", po::value<std::vector<std::string>>(), "Search folders.")
            ("file-stems,s", po::value<std::vector<std::string>>(), "File stems.")
            ("extensions,e", po::value<std::vector<std::string>>(), "File extensions.")
            ("regexp,r", po::value<std::vector<std::string>>(), "Search using regular expression.")
            ("database,d", po::value<std::string>(), "File database.")
            ("create-database,c", po::value<std::string>(), "Create database.")
            ("ignore-case,i", po::value<std::string>(), "Ignore case distinctions when matching patterns.");
        
        po::positional_options_description p;
        p.add("folder", -1);
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << "Usage: finder [options]\n";
            std::cout << desc;
            std::cout << "Examples:" << std::endl;
            std::cout << "\t finder ./ -o testdata." << std::endl;
        } else {
            if (vm.count("folder")) {
                Folders = vm["folder"].as<std::vector<std::string>>();
            } else {
                Folders.emplace_back(boost::filesystem::current_path().string());
            }

            if (vm.count("database")) {
                Database = vm["database"].as<std::string>();
            } else {
                Database = "data";
            }
        }
    }


      void disp() {
          std::cout << "Verbose: " << (Verbose ? ("true") : ("false")) << std::endl;
          std::cout << "Ignore case: " << (IgnoreCase ? "true" : "false") << std::endl;
          std::cout << "Count only: " << (CountOnly ? "true" : "false") << std::endl;
          std::cout << "Database: \"" << Database << "\"\n";
          std::cout << "Search folders: ";
          for (auto & val : Folders) std::cout << "\"" << val << "\" ";
          std::cout << std::endl;
          std::cout << "Search pattern: \"" << SearchPattern << "\"\n";
      }
      
      // Member variables
      bool Verbose;
      bool IgnoreCase;
      bool CountOnly;
      size_t Limit;
      std::vector<std::string> Folders;
      std::string SearchPattern;
      std::string Database;
  };
}
#endif
