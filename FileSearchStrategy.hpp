#ifndef FileSearchStrategy_hpp_
#define FileSearchStrategy_hpp_

#include "boost/filesystem.hpp"
#include <type_traits>
#include <unordered_map>
#include "boost/regex.hpp"

namespace Tools
{
  namespace FileTypeTag
  {
    struct All;
    struct Edited;
    struct OwnerReadOnly;
  }

  namespace FileExtensionTag
  {
    struct All;
    struct Specific;
  }

  namespace FileNameTag
  {
    struct All;
    struct Specific;
  }

  // File type search strategy
  template<typename T> struct FileSearchStrategy;


  template<>
  struct FileSearchStrategy<FileTypeTag::All>
  {
    static bool apply(boost::filesystem::file_status & fs)
    {
      return (fs.type() == boost::filesystem::regular_file);
    }
  };


  template<>
  struct FileSearchStrategy<FileTypeTag::Edited>
  {
    static bool apply(boost::filesystem::file_status & fs)
    {
      const boost::filesystem::perms fperm = fs.permissions();
      return ((fs.type() == boost::filesystem::regular_file) && (fperm & boost::filesystem::owner_write));
    }
  };


  template<>
  struct FileSearchStrategy<FileTypeTag::OwnerReadOnly>
  {
    static bool apply(boost::filesystem::file_status & fs)
    {
      const boost::filesystem::perms fperm = fs.permissions();
      return ((fs.type() == boost::filesystem::regular_file) && (fperm == boost::filesystem::owner_read));
    }
  };


  // File extension search strategy
  template<typename T, typename ExtMap> struct FileExtension;
  template<typename ExtMap>
  struct FileExtension<FileExtensionTag::All, ExtMap>
  {
    static bool apply(const std::string &, const ExtMap &)
    {
      return true;
    }
  };


  template<typename ExtMap>
  struct FileExtension<FileExtensionTag::Specific, ExtMap>
  {
    static bool apply(const std::string & ext, const ExtMap & supportedExtensions)
    {
      return supportedExtensions.find(ext) != supportedExtensions.end();
    }
  };


  /**
   * File search strategies
   * 
   * @note boost::regex is used to match file name pattern. 
   */

  template<typename T> struct FileName;


  template<>
  struct FileName<FileNameTag::All> 
  {
    static bool apply(const std::string &, const boost::regex &) 
    {
      return true;
    }
  };
    

  template<>
  struct FileName<FileNameTag::Specific> 
  {
    static bool apply(const std::string & fileName, const boost::regex & expression) 
    {
      return boost::regex_match(fileName, expression);
    }
  };
}
#endif
