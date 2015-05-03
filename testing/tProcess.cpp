#include <fstream>
#include <iostream>
#include "utils/Process.hpp"

int main() {
  using Poco::Process;
  using Poco::ProcessHandle;
  std::string cmd("/bin/ls");
  std::vector<std::string> args = {"-la"};
  std::string buffer;
  Tools::run(cmd, args, buffer);
  std::cout << buffer;
  std::cout << Tools::run(cmd, args);
    
  return 0;
}
