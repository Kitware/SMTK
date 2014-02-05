#include "smtk/cgm/ExportSolid.h"
#include "smtk/cgm/TDUUID.h"

#include "smtk/model/Cursor.h"
#include "smtk/model/Storage.h"

#include "CubitCompat.hpp"
#include "RefEntity.hpp"

namespace cgmsmtk {
  namespace cgm {

/**\brief Export the specified entities to the given file.
  *
  * You must also specify the file type, but be aware that it
  * should match the type used to load the model.
  */
int ExportSolid::entitiesToFileOfNameAndType(
  const smtk::model::CursorArray& entities,
  const std::string& filename,
  const std::string& filetype)
{
  DLIList<RefEntity*> refsOut;
  smtk::model::CursorArray::const_iterator it;
  for (it = entities.begin(); it != entities.end(); ++it)
    {
    RefEntity* matchingCGMEnt = dynamic_cast<RefEntity*>(
      TDUUID::findEntityById(it->entity()));
    if (matchingCGMEnt)
      refsOut.append(matchingCGMEnt);
    }
  if (refsOut.size() <= 0)
    return 1;

  int num_exported;
  CubitString version;
  CubitStatus s;
  s = CubitCompat_export_solid_model(
    refsOut, filename.c_str(), filetype.c_str(),
    num_exported, version, /*logfile_name*/ NULL);
  return s == CUBIT_SUCCESS ? 0 : 1;
}

  } // namespace cgm
} // namespace cgmsmtk
