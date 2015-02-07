#ifdef _WIN64
template <> const char FileSeparator<char>::value = '\\';
#elif _WIN32
template <> const char FileSeparator<char>::value = '\\';
#elif __APPLE__
template <> const char FileSeparator<char>::value = '/';
#elif __linux
template <> const char FileSeparator<char>::value = '/';
#elif __unix
template <> const char FileSeparator<char>::value = '/';
#endif


#ifdef _WIN64
template <> const std::string FileSeparator<std::string>::value = "\\";
#elif _WIN32
template <> const std::string FileSeparator<std::string>::value = "\\";
#elif __APPLE__
template <> const std::string FileSeparator<std::string>::value = "/";
#elif __linux
template <> const std::string FileSeparator<std::string>::value = "/";
#elif __unix
template <> const std::string FileSeparator<std::string>::value = "/";
#endif


#ifdef _WIN64
template <> const std::string TemporaryFolder<std::string>::value = "C:\\tmp\\"; // TODO: Fix this
#elif _WIN32
template <> const std::string TemporaryFolder<std::string>::value = "C:\\tmp\\"; // TODO: Fix this
#elif __APPLE__
template <> const std::string TemporaryFolder<std::string>::value = "/tmp"; // TODO: Check this
#elif __linux
template <> const std::string TemporaryFolder<std::string>::value = "/tmp";
template <> const std::string Resources<std::string>::FileSeparator = "/";
template <> const std::string Resources<std::string>::TemporaryFolder = "/tmp";
template <> const std::string Resources<std::string>::PrintFolder = "~/print";
#elif __unix
template <> const std::string TemporaryFolder<std::string>::value = "/tmp";
#endif
