#ifndef Resources_hpp_
#define Resources_hpp_

#include <string>

namespace utils {
    struct Resources {
        static const std::string Database;
      static const std::string Info;
        static const std::string GraphKey;
        static const std::string VertexKey;
        static const std::string EdgeKey;
        static const std::string VIDKey;
        static const std::string AllFileKey;
    };
    const std::string Resources::Database = ".database";
  const std::string Resources::Info = "_info_";
    const std::string Resources::GraphKey = "graph_data";
    const std::string Resources::VertexKey = "v";
    const std::string Resources::EdgeKey = "e";
    const std::string Resources::VIDKey = "vids";
    const std::string Resources::AllFileKey = "all_files";
}

#endif
