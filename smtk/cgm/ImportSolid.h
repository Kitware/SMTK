#ifndef __smtk_cgm_ImportSolid_h
#define __smtk_cgm_ImportSolid_h

#include "smtk/cgmSMTKExports.h" // for CGMSMTK_EXPORT
#include "smtk/PublicPointerDefs.h" // For ManagerPtr

#include "smtk/util/UUID.h"

namespace smtk {
  namespace model {
    class Manager;
  }
}

namespace cgmsmtk {
  namespace cgm {

/**\brief Load a solid model using CGM.
  *
  */
class CGMSMTK_EXPORT ImportSolid
{
public:
  static smtk::util::UUIDArray fromFilenameIntoManager(
    const std::string& filename,
    const std::string& filetype,
    smtk::model::ManagerPtr manager);
};

  } // namespace cgm
} // namespace cgmsmtk

#endif // __smtk_cgm_ImportSolid_h
