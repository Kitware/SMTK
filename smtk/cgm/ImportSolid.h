#ifndef __smtk_cgm_ImportSolid_h
#define __smtk_cgm_ImportSolid_h

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
class CGMSMTK_EXPORT ImportSolid
{
public:
  static smtk::util::UUID fromFileNameIntoStorage(
    const std::string& filename,
    const std::string& filetype,
    smtk::model::StoragePtr storage);
};

  } // namespace cgm
} // namespace cgmsmtk

#endif // __smtk_cgm_ImportSolid_h
