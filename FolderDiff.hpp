#ifndef FolderDiff_hpp
#define FolderDiff_hpp

namespace Utils {
    template <typename Container> class FolderDiff {
      public:
        template <typename FileSearch>
        void find(FileSearch &fsearch, const std::string &aFolder) {
            fsearch.search(aFolder);
        }

        template <typename Reader>
        Container read(Reader &reader, const std::string &aKey) {
            Container dict;
            auto const results = reader.read(aKey);
            if (!results.empty()) {
                std::istringstream is(results);
                Utils::load<Utils::IArchive, Container>(dict, is);
            }
            return dict;
        }

      /**
       * This methods will return a tuple which has
       *     1. Items in first and not in second.
       *     2. Items in both first and second but they are different.
       *     3. Items which are in second and not in first.
       * The first item of each data element is a using string which is a full file name. 
       */
        std::tuple<Container, Container, Container>
        diff(const Container &first, const Container &second) {

          Container modifiedFiles;
          Container newFiles;
          Container deletedFiles;

          // Create a lookup table
          typedef typename Container::value_type value_type;
          std::set<value_type> dict;
          for (auto item : second) {
            dict.emplace(item);
          }

          // Cleanup iteam which belong to both.
          // The complexity of this algorithm is n * log(n).
          for (auto item : first) {
            auto pos = dict.find(item);
            if (pos == dict.end()) {
              modifiedFiles.emplace_back(item);
            } else {
              dict.erase(pos);
            }
          }

          // Separate modified, new, and delete files.
          
          std::cout << "----\n";
          Utils::print(modifiedFiles);
          std::cout << "----\n";
          Utils::print(dict);

          // Get a list of edited, new, and removed files.
          
          
          return std::make_tuple(modifiedFiles, newFiles, deletedFiles);
        }
    };
}
#endif
