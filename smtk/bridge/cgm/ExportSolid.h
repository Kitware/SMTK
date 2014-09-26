#ifndef __smtk_bridge_cgm_ExportSolid_h
#define __smtk_bridge_cgm_ExportSolid_h

#include "smtk/bridge/cgm/cgmSMTKExports.h" // for CGMSMTK_EXPORT
#include "smtk/PublicPointerDefs.h" // For ManagerPtr

#include "smtk/common/UUID.h"

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
class CGMSMTK_EXPORT ExportSolid
{
public:
  static int entitiesToFileOfNameAndType(
    const std::vector<smtk::model::Cursor>& entities,
    const std::string& filename,
    const std::string& filetype);
};

} // namespace cgm
  } //namespace bridge
} // namespace smtk

#endif // __smtk_bridge_cgm_ExportSolid_h
