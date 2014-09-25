#include "smtk/bridge/cgm/Engines.h"

#include <algorithm>
#include <iostream>

using namespace smtk::bridge::cgm;

typedef std::vector<std::string> StringArray;

int main(int argc, char* argv[])
{
  bool passed = true;
  bool testSetDefault = false;
  for (int i = 1; i < argc; ++i)
    {
    std::string arg(argv[i]);
    if (arg == "-test-set-default")
      testSetDefault = true;
    }
  StringArray engines = Engines::listEngines();
  std::cout << "### Modeling kernels built into this CGM library ###\n\n";
  for (StringArray::iterator it = engines.begin(); it != engines.end(); ++it)
    std::cout << "+ " << *it << "\n";
  std::cout << "\n";

  if (testSetDefault)
    {
    for (StringArray::iterator it = engines.begin(); it != engines.end(); ++it)
      {
      std::string engine(*it);
      std::transform(engine.begin(), engine.end(), engine.begin(),
        std::bind2nd(std::ptr_fun(&std::tolower<char>), std::locale("")));

      // The FACET engine cannot be the default. (It is unclear why this is so.)
      if (engine == "facet" || engine == "(null)")
        continue;

      if (!Engines::setDefault(*it))
        {
        std::cout << "Could not make \"" << *it << "\" engine the default.\n";
        passed = false;
        }
      }
    }

  // Uncomment when CGM no longer dies trying to shut down.
  // This happens on OS X 10.9, x86_64, OCC.
  // Engines::shutdown();

  return passed ? 0 : -1;
}
