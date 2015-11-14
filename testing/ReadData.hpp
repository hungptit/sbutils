#ifndef ReadData_hpp_
namespace {
    template<typename Reader, typename IArchive, typename Container>
    class ReadData {
      public:
        ReadData(const std::string &dataFile) : DBConnector(dataFile) {}
        Container read(const std::vector<std::string> &keys) {
            Container results;
            for (auto & aKey : keys) {
                decltype(results) data;
                std::istringstream is(DBConnector.read(aKey));
                Tools::load<IArchive, decltype(data)>(data, is);
                std::move(data.begin(), data.end(),
                          std::back_inserter(results));
            }
            return results;
        }

      private:
        Reader DBConnector;
    };

    template<typename Reader, typename IArchive, typename Container>
    class WriteData {
      public:
        WriteData(const std::string &dataFile) : DBConnector(dataFile) {}
        Container write(const std::vector<std::string> &keys) {
            Container results;
            for (auto & aKey : keys) {
                decltype(results) data;
                std::istringstream is(DBConnector.read(aKey));
                Tools::load<IArchive, decltype(data)>(data, is);
                std::move(data.begin(), data.end(),
                          std::back_inserter(results));
            }
            return results;
        }

      private:
        Writer DBConnector;
    };
}
#endif
