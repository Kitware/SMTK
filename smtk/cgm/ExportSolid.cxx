#include "smtk/cgm/ExportSolid.h"

#include "smtk/model/Storage.h"


namespace cgmsmtk {
  namespace cgm {

smtk::util::UUID ExportSolid::fromFileNameIntoStorage(
  const std::string& filename, smtk::model::StoragePtr storage)
{
  (void)storage;
  std::cout << "Export from file " << filename.c_str() << " not implemented.\n";
  return smtk::util::UUID::null();
}

  } // namespace cgm
} // namespace cgmsmtk
