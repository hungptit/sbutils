#ifndef Basic_hpp_
#define Basic_hpp_

#include <iostream>
#include <string>
#include <cstdlib>

namespace Tools
{
  template <typename Data>
  void disp(Data & data, const std::string & message = "");

  template <typename Data>
  void disp_pair(Data & data, const std::string & message = "");

  int run(const std::string & command);

  std::string getenv(const std::string & variableName);

#include "private/Basic.cpp"
}

#endif
