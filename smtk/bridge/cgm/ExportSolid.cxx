//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/ExportSolid.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"

#include "CGMApp.hpp"
#include "CubitAttribManager.hpp"
#include "CubitCompat.hpp"
#include "RefEntity.hpp"

namespace smtk
{
namespace bridge
{
namespace cgm
{

/**\brief Export the specified entities to the given file.
  *
  * You must also specify the file type, but be aware that it
  * should match the type used to load the model.
  */
int ExportSolid::entitiesToFileOfNameAndType(const smtk::model::EntityRefArray& entities,
  const std::string& filename, const std::string& filetype)
{
  DLIList<RefEntity*> refsOut;
  smtk::model::EntityRefArray::const_iterator it;
  for (it = entities.begin(); it != entities.end(); ++it)
  {
    RefEntity* matchingCGMEnt = dynamic_cast<RefEntity*>(TDUUID::findEntityById(it->entity()));
    if (matchingCGMEnt)
      refsOut.append(matchingCGMEnt);
  }
  if (refsOut.size() <= 0)
    return 1;

  int num_exported;
  CubitString version;
  CubitStatus s;
  int prevAutoFlag = CGMApp::instance()->attrib_manager()->auto_flag();
  CGMApp::instance()->attrib_manager()->auto_flag(CUBIT_TRUE);
  s = CubitCompat_export_solid_model(
    refsOut, filename.c_str(), filetype.c_str(), num_exported, version, /*logfile_name*/ NULL);
  CGMApp::instance()->attrib_manager()->auto_flag(prevAutoFlag);
  return s == CUBIT_SUCCESS ? 0 : 1;
}

} // namespace cgm
} //namespace bridge
} // namespace smtk
