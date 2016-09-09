#pragma once

#include <string>

namespace utils {
    struct Resources {
        static const std::string Database;
        static const std::string Info;
        static const std::string GraphKey;
        static const std::string VertexKey;
        static const std::string VIDKey;
        static const std::string EdgeKey;
        static const std::string AllFileKey;
    };
    const std::string Resources::Database = ".database";
    const std::string Resources::Info = "_info_";
    const std::string Resources::GraphKey = "_graph_";
    const std::string Resources::VertexKey = "_vertexes_";
    const std::string Resources::VIDKey = "_vids_";
    const std::string Resources::EdgeKey = "_edges_";
    const std::string Resources::AllFileKey = "_files_";
}
