#ifndef Resources_hpp_
#define Resources_hpp_
namespace Tools
{
    template <typename T> 
    struct FileSeparator
    {
        typedef T value_type;
        static const T value;
    };


    template <typename T>
    struct TemporaryFolder
    {
        typedef T value_type;
        static const T value;
    };
    
    
    template <typename T>
    struct Resources
    {
        typedef T value_type;
        static const T TemporaryFolder;
        static const T PrintFolder; 
        static const T FileSeparator;
    };
    
    #include "private/Resources.cpp"
}
#endif
