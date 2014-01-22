#include "smtk/cgm/CAUUID.h"
#include "smtk/cgm/ImportSolid.h"
#include "smtk/cgm/TDUUID.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Storage.h"

#include "smtk/util/UUID.h"

#include "DagType.hpp"
#include "CubitCompat.hpp"
#include "CubitDefines.h"
#include "DLIList.hpp"
#include "InitCGMA.hpp"
#include "GeometryModifyTool.hpp"
#include "GeometryQueryTool.hpp"
#include "RefEntity.hpp"
#include "RefEntityFactory.hpp"
#include "RefGroup.hpp"

#include <map>

using namespace smtk::model;

namespace cgmsmtk {
  namespace cgm {

smtk::util::UUID ImportSolid::fromFileNameIntoStorage(
  const std::string& filename,
  const std::string& filetype,
  smtk::model::StoragePtr storage)
{
  cgmsmtk::cgm::CAUUID::registerWithAttributeManager();
  std::string engine = "OCC";
  if (filetype == "FACET_TYPE") engine = "FACET";
  else if (filetype == "ACIS_SAT") engine = "ACIS";
  CubitStatus s = InitCGMA::initialize_cgma(engine.c_str());
  if (s != CUBIT_SUCCESS)
    {
    return smtk::util::UUID::null();
    }
  DLIList<RefEntity*> imported;
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
 if (s != CUBIT_SUCCESS)
   {
   std::cerr << "Failed to import CGM model, status " << s << "\n";
   return smtk::util::UUID::null();
   }

 // Create a model and a matching CGM RefGroup that will "model" the model.
 std::string modelName = filename.substr(0, filename.find_last_of("."));
 RefGroup* mg =
   RefEntityFactory::instance()->construct_RefGroup(modelName.c_str());
 smtk::util::UUID mid = cgmsmtk::cgm::TDUUID::ofEntity(mg, true)->entityId();
 smtk::model::ModelEntity me = storage->insertModel(mid, 3, 3, modelName);

 int ne = static_cast<int>(imported.size());
 for (int i = 0; i < ne; ++i)
   {
   RefEntity* entry = imported.get_and_step();
   cgmsmtk::cgm::TDUUID* refId = cgmsmtk::cgm::TDUUID::ofEntity(entry, true);
   smtk::util::UUID entId = refId->entityId();
   // Eventually replace with:
   // Cursor smtkEntry = Bridge::AddCGMEntityToStorage(entId, storage);
   if (entry->dag_type() == DagType::body_type())
     {
     UUIDWithEntity iter = storage->setEntityOfTypeAndDimension(entId, MODEL_ENTITY, 3);
     me.addSubmodel(smtk::model::ModelEntity(storage, iter->first));
     }
   else if (entry->dag_type().functional_type() == DagType::BasicTopologyEntity_TYPE)
     {
     //std::cout << i << " cls " << entry->class_name() << " d " << entry->dimension() << "\n";
     UUIDWithEntity iter = storage->setCellOfDimension(entId, entry->dimension());
     me.addCell(CellEntity(storage, iter->first));
     }
   }
 // Add the same entities to the model
 imported.reset();
 mg->add_ref_entity(imported);

  return me.entity();
}

  } // namespace cgm
} // namespace cgmsmtk
