#ifndef __smtk_common_Environment_h
#define __smtk_common_Environment_h

#include "smtk/SMTKCoreExports.h"

#include <string>

namespace smtk {
  namespace common {

/**\brief Cross-platform access to process environment variables.
  *
  */
class SMTKCORE_EXPORT Environment
{
public:
  static bool hasVariable(const std::string& varName);
  static std::string getVariable(const std::string& varName);
  static void setVariable(const std::string& varName, const std::string& value);
};

  } // namespace common
} // namespace smtk

#endif // __smtk_common_Environment_h
