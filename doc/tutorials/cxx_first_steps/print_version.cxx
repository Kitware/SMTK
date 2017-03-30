// ++ 1 ++
#include <iostream>

#include "smtk/common/Version.h"

int main()
{
  std::cout << "Compiled with SMTK version " << smtk::common::Version::number() << "\n";
  return 0;
}
// -- 1 --
