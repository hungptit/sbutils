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

        // clang-format off
        desc.add_options()              
            ("help,h", "Print this help")
            ("verbose,v", "Display searched data.")
            ("folders,f", po::value<std::vector<std::string>>(), "Search folders.")
            ("file-stems,s", po::value<std::vector<std::string>>(), "File stems.")
            ("extensions,e", po::value<std::vector<std::string>>(), "File extensions.")
            ("search-strings,t", po::value<std::vector<std::string>>(), "File extensions.")
            ("regexp,r", po::value<std::vector<std::string>>(), "Search using regular expression.")
            ("database,d", po::value<std::string>(), "File database.")
            ("create-database,c", po::value<std::string>(), "Create database.")
            ("ignore-case,i", po::value<std::string>(), "Ignore case distinctions when matching patterns.");
        // clang-format on

        po::positional_options_description p;
        p.add("folders", -1);
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            Help = true;
            std::cout << "Usage: finder [options]\n";
            std::cout << desc;
            std::cout << "Examples:" << std::endl;
            std::cout << "\t finder ./ -d testdata." << std::endl;
        } else {
            Help = false;

            if (vm.count("ignore-case")) {
                IgnoreCase = true;
            }
            else {
                IgnoreCase = false;
            }

            if (vm.count("verbose")) {
                Verbose = true;
            }
            else {
                Verbose = false;
            }

            if (vm.count("folders")) {
                Folders = vm["folders"].as<std::vector<std::string>>();
            } else {
                Folders.emplace_back(boost::filesystem::current_path().string());
            }

            if (vm.count("file-stems")) {
                Stems = vm["file-stems"].as<std::vector<std::string>>();
            }

            if (vm.count("extensions")) {
                Extensions = vm["extensions"].as<std::vector<std::string>>();
            }

            if (vm.count("search-strings")) {
                SearchStrings = vm["search-strings"].as<std::vector<std::string>>();
            }
            
            
            if (vm.count("database")) {
                Database = vm["database"].as<std::string>();
            }
        }
    }


      void disp() {
          std::cout << "Verbose: " << (Verbose ? ("true") : ("false")) << std::endl;
          std::cout << "Ignore case: " << (IgnoreCase ? "true" : "false") << std::endl;
          std::cout << "Count only: " << (CountOnly ? "true" : "false") << std::endl;
          std::cout << "Database: \"" << Database << "\"\n";
          std::cout << "Search folders: ";
          
          // Display searched folders
          for (auto & val : Folders) std::cout << "\"" << val << "\" ";
          std::cout << std::endl;

          // Display searched extensions
          if (!Extensions.empty()) {
              std::cout << "Search extensions: ";
              for (auto & val : Folders) std::cout << "\"" << val << "\" ";
          }
          std::cout << std::endl;

          // Display searched patterns
          if (!SearchPattern.empty()) {
              std::cout << "Search pattern: \"" << SearchPattern << "\"\n";
          }          
      }
      
      // Member variables
      bool Help;
      bool Verbose;
      bool IgnoreCase;
      bool CountOnly;
      size_t Limit;
      std::vector<std::string> Folders;
      std::vector<std::string> Extensions;
      std::vector<std::string> Stems;
      std::vector<std::string> SearchStrings;
      std::string SearchPattern;
      std::string Database;
  };
}
#endif
