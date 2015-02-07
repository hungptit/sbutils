bool isRegularFile(const std::string & str)
{
    const boost::filesystem::path file(str);
    return boost::filesystem::is_regular_file(file);
}


bool isDirectory(const std::string & folderPath)
{
    const boost::filesystem::path file(folderPath);
    return boost::filesystem::is_directory(file);
}


bool createDirectory(const std::string & folderPath)
{
    const boost::filesystem::path folder(folderPath);
    if (boost::filesystem::is_directory(folder))
        return true;
    else
        return boost::filesystem::create_directories(folder);
}


bool remove(const std::string & folderName)
{
    const boost::filesystem::path p(folderName);
    if (boost::filesystem::exists(p))
    {
        if (boost::filesystem::is_directory(p))
        {
            return boost::filesystem::remove_all(p);
        }
        else
        {
            return boost::filesystem::remove(p);
        }
    }
    return false;
}


const std::string getCurrentFolder()
{
    const boost::filesystem::path fullPath = boost::filesystem::current_path();
    std::string fullPathName = fullPath.string();
    return fullPathName;
}


std::string getAbslutePath(const std::string & pathName)
{
    boost::filesystem::path path(pathName);
    boost::filesystem::path fullPath = boost::filesystem::canonical(path);
    std::string fullPathName = fullPath.string();
    return fullPathName;
}
