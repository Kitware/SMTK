#include "smtk/cgm/ImportSolid.h"

#include "smtk/cgm/Bridge.h"
#include "smtk/cgm/CAUUID.h"
#include "smtk/cgm/Engines.h"
#include "smtk/cgm/TDUUID.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Manager.h"

#include "smtk/util/UUID.h"

#include "CGMApp.hpp"
#include "DagType.hpp"
#include "CubitAttribManager.hpp"
#include "CubitCompat.hpp"
#include "CubitDefines.h"
#include "DLIList.hpp"
#include "InitCGMA.hpp"
#include "GeometryModifyTool.hpp"
#include "GeometryQueryEngine.hpp"
#include "GeometryQueryTool.hpp"
#include "RefEntity.hpp"
#include "RefEntityFactory.hpp"
#include "RefGroup.hpp"

#include <map>

using namespace smtk::model;

namespace cgmsmtk {
  namespace cgm {

smtk::util::UUIDArray ImportSolid::fromFilenameIntoManager(
  const std::string& filename,
  const std::string& filetype,
  smtk::model::ManagerPtr manager)
{
  smtk::util::UUIDArray result;
  cgmsmtk::cgm::CAUUID::registerWithAttributeManager();
  std::string engine = "OCC";
  if (filetype == "FACET_TYPE") engine = "FACET";
  else if (filetype == "ACIS_SAT") engine = "ACIS";
  if (!Engines::setDefault(engine))
    {
    std::cerr << "Could not set default engine to \"" << engine << "\"\n";
    return result;
    }
  std::cout << "Default modeler now \"" << GeometryQueryTool::instance()->get_gqe()->modeler_type() << "\"\n";
  CubitStatus s;
  DLIList<RefEntity*> imported;
  int prevAutoFlag = CGMApp::instance()->attrib_manager()->auto_flag();
  CGMApp::instance()->attrib_manager()->auto_flag(CUBIT_TRUE);
  s = CubitCompat_import_solid_model(
    filename.c_str(),
    filetype.c_str(),
    /*logfile_name*/ NULL,
    /*heal_step*/ CUBIT_TRUE,
    /*import_bodies*/ CUBIT_TRUE,
    /*import_surfaces*/ CUBIT_TRUE,
    /*import_curves*/ CUBIT_TRUE,
    /*import_vertices*/ CUBIT_TRUE,
    /*free_surfaces*/ CUBIT_TRUE,
    &imported
  );
  CGMApp::instance()->attrib_manager()->auto_flag(prevAutoFlag);
  if (s != CUBIT_SUCCESS)
    {
    std::cerr << "Failed to import CGM model, status " << s << "\n";
    return result;
    }

  Bridge::Ptr bridge = Bridge::create();
  std::string modelName = filename.substr(0, filename.find_last_of("."));
  int ne = static_cast<int>(imported.size());
  result.reserve(ne);
  for (int i = 0; i < ne; ++i)
    {
    RefEntity* entry = imported.get_and_step();
    cgmsmtk::cgm::TDUUID* refId = cgmsmtk::cgm::TDUUID::ofEntity(entry, true);
    smtk::util::UUID entId = refId->entityId();
    Cursor smtkEntry(manager, entId);
    if (bridge->transcribe(smtkEntry, BRIDGE_EVERYTHING, false))
      result.push_back(smtkEntry.entity());
    }
  // FIXME: Until this is implemented, Bridge will be deleted upon exit:
  //manager->addBridge(bridge);
  imported.reset();

  return result;
}

  } // namespace cgm
} // namespace cgmsmtk
