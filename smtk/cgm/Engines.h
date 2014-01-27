#ifndef __smtk_cgm_Engines_h
#define __smtk_cgm_Engines_h

#include "smtk/options.h" // for CGM_HAVE_VERSION_H
#include "smtk/cgmSMTKExports.h"
#ifdef CGM_HAVE_VERSION_H
#  include "cgm_version.h"
#endif

#include <string>
#include <vector>

namespace cgmsmtk {
  namespace cgm {

/**\brief Ensure that CGMA has been initialized.
  *
  * This registers attributes with CGM and prepares one
  * GeometryQueryEngine to be the default.
  * You may change the default engine later.
  */
class CGMSMTK_EXPORT Engines
{
public:
  static bool areInitialized();
  static bool isInitialized(const std::string& engine, const std::vector<std::string>& args = std::vector<std::string>());
  static bool setDefault(const std::string& engine);
  static bool shutdown();
};

  } // namespace cgm
} // namespace cgmsmtk

#endif // __smtk_cgm_Engines_h
