#include "smtk/cgm/ImportSolid.h"

#include "smtk/model/Storage.h"


namespace cgmsmtk {
  namespace cgm {

smtk::util::UUID ImportSolid::fromFileNameIntoStorage(
  const std::string& filename, smtk::model::StoragePtr storage)
{
  (void)storage;
  std::cout << "Import from file " << filename.c_str() << " not implemented.\n";
  return smtk::util::UUID::null();
}

  } // namespace cgm
} // namespace cgmsmtk
