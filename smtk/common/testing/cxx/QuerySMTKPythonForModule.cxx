//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/PythonInterpreter.h"

#include <iostream>

// Using SMTK's heuristics for locating python modules, query the embedded
// python interpreter for a module.

int main(int argc, char* argv[])
{
  std::string module = "smtk";

  if (argc > 1)
  {
    module = std::string(argv[1]);
  }

  bool hasModule = smtk::common::PythonInterpreter::instance().canFindModule(module);

  if (hasModule)
  {
    std::cout << module << " found" << std::endl;
    return 0;
  }
  else
  {
    std::cout << module << " not found" << std::endl;
    std::cout << std::endl << "Paths searched: " << std::endl;

    auto pathVec = smtk::common::PythonInterpreter::instance().pythonPath();
    for (auto path : pathVec)
    {
      std::cout << path << std::endl;
    }
    return 1;
  }
}
