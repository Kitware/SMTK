//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/ImportSolid.h"

#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/Session.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "smtk/common/UUID.h"

#include "CGMApp.hpp"
#include "CubitAttribManager.hpp"
#include "CubitCompat.hpp"
#include "CubitDefines.h"
#include "DLIList.hpp"
#include "DagType.hpp"
#include "GeometryModifyTool.hpp"
#include "GeometryQueryEngine.hpp"
#include "GeometryQueryTool.hpp"
#include "InitCGMA.hpp"
#include "RefEntity.hpp"
#include "RefEntityFactory.hpp"
#include "RefGroup.hpp"

#include <map>

using namespace smtk::model;

namespace smtk
{
namespace bridge
{
namespace cgm
{

smtk::common::UUIDArray ImportSolid::fromFilenameIntoManager(
  const std::string& filename, const std::string& filetype, smtk::model::ManagerPtr manager)
{
  smtk::common::UUIDArray result;
  smtk::bridge::cgm::CAUUID::registerWithAttributeManager();
  std::string engine = "OCC";
  if (filetype == "FACET_TYPE")
    engine = "FACET";
  else if (filetype == "ACIS_SAT")
    engine = "ACIS";
  if (!Engines::setDefault(engine))
  {
    std::cerr << "Could not set default engine to \"" << engine << "\"\n";
    return result;
  }
  std::cout << "Default modeler now \"" << GeometryQueryTool::instance()->get_gqe()->modeler_type()
            << "\"\n";
  CubitStatus s;
  DLIList<RefEntity*> imported;
  int prevAutoFlag = CGMApp::instance()->attrib_manager()->auto_flag();
  CGMApp::instance()->attrib_manager()->auto_flag(CUBIT_TRUE);
  s = CubitCompat_import_solid_model(filename.c_str(), filetype.c_str(),
    /*logfile_name*/ NULL,
    /*heal_step*/ CUBIT_TRUE,
    /*import_bodies*/ CUBIT_TRUE,
    /*import_surfaces*/ CUBIT_TRUE,
    /*import_curves*/ CUBIT_TRUE,
    /*import_vertices*/ CUBIT_TRUE,
    /*free_surfaces*/ CUBIT_TRUE, &imported);
  CGMApp::instance()->attrib_manager()->auto_flag(prevAutoFlag);
  if (s != CUBIT_SUCCESS)
  {
    std::cerr << "Failed to import CGM model, status " << s << "\n";
    return result;
  }

  Session::Ptr session = Session::create();
  std::string modelName = filename.substr(0, filename.find_last_of("."));
  int ne = static_cast<int>(imported.size());
  result.reserve(ne);
  for (int i = 0; i < ne; ++i)
  {
    RefEntity* entry = imported.get_and_step();
    smtk::bridge::cgm::TDUUID* refId = smtk::bridge::cgm::TDUUID::ofEntity(entry, true);
    smtk::common::UUID entId = refId->entityId();
    EntityRef smtkEntry(manager, entId);
    if (session->transcribe(smtkEntry, SESSION_EVERYTHING, false))
      result.push_back(smtkEntry.entity());
  }
  // FIXME: Until this is implemented, Session will be deleted upon exit:
  //manager->addSession(session);
  imported.reset();

  return result;
}

} // namespace cgm
} //namespace bridge
} // namespace smtk
