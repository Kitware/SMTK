#ifndef __smtk_cgm_ExportSolid_h
#define __smtk_cgm_ExportSolid_h

#include "smtk/cgmSMTKExports.h" // for CGMSMTK_EXPORT
#include "smtk/PublicPointerDefs.h" // For StoragePtr

#include "smtk/util/UUID.h"

namespace smtk {
  namespace model {
    class Storage;
  }
}

namespace cgmsmtk {
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
} // namespace cgmsmtk

#endif // __smtk_cgm_ExportSolid_h
