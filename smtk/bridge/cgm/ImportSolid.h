#ifndef __smtk_bridge_cgm_ImportSolid_h
#define __smtk_bridge_cgm_ImportSolid_h

#include "smtk/bridge/cgm/cgmSMTKExports.h" // for CGMSMTK_EXPORT
#include "smtk/PublicPointerDefs.h" // For ManagerPtr

#include "smtk/util/UUID.h"

namespace smtk {
  namespace model {
    class Manager;
  }
}

namespace smtk {
  namespace bridge {
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
  } //namespace bridge
} // namespace smtk

#endif // __smtk_bridge_cgm_ImportSolid_h
