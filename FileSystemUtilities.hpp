#ifndef FileSystemUtilities_hpp_
#define FileSystemUtilities_hpp_
#include "boost/filesystem.hpp"
#include <string>
namespace Tools 
{
    bool createDirectory(const std::string & folderName);
    bool remove(const std::string & folderName);
    bool isRegularFile(const std::string & str);    
    bool isDirectory(const std::string & str);      
    const std::string getCurrentFolder();
    std::string getAbslutePath(const std::string & path);       

#include "private/FileSystemUtilities.cpp"
}
#endif
