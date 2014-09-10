#include "smtk/remote/RemusRemoteBridge.h"

#include "smtk/options.h"

#include "smtk/util/AutoInit.h"

#ifdef SMTK_USE_CGM
// If CGM is included in the build, ensure that it is loaded
// (and thus registered with the remus remote bridge).
smtkComponentInitMacro(smtk_cgm_bridge);
#endif // SMTK_USE_CGM

using namespace smtk::model;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  StringList typeNames = smtk::model::RemusRemoteBridge::availableTypeNames();
  for (StringList::const_iterator it = typeNames.begin(); it != typeNames.end(); ++it)
    {
    std::cout
      << "{\n"
      << "  \"InputType\":\"smtk\",\n"
      << "  \"OutputType\":\"" << *it << "\",\n"
      << "  \"ExecutableName\":\"../../../bin/smtk-remote-worker\",\n"
      << "}\n"
      ;
    }
  return 0;
}
