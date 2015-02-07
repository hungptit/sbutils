template <typename Data>    
void disp(Data & data, const std::string & message)
{
    std::cout << message << "[ ";
    for (const auto & val: data)
        std::cout << val << " ";    
    std::cout << "]" << std::endl;
}


template <typename Data>    
void disp_pair(Data & data, const std::string & message)
{
    std::cout << message << "[ ";
    for (const auto & val : data)
        std::cout << "(" << val.first << "," << val.second << ") ";         
    std::cout << "]" << std::endl;
}


/** 
 * @todo Is this function portable?
 * 
 * @param command 
 * 
 * @return 
 */
int run(const std::string & command)
{
    return std::system(command.c_str());
}


/** 
 * @todo This function is not portable. How to make it work on Windows? 
 * 
 * @param variableName 
 * 
 * @return A std::string which has environment variable's value
 */
std::string getEnvironmentVariableValue(const std::string & variableName)
{
    char * retval = std::getenv(variableName.c_str());
    if (retval == NULL)
    {
        std::string results;
        return results;
    }
    else
    {
        std::string results(retval);
        return results;
    }
}
