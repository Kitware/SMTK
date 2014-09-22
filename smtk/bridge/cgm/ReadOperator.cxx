#include "smtk/cgm/ReadOperator.h"

#include "smtk/cgm/Bridge.h"
#include "smtk/cgm/CAUUID.h"
#include "smtk/cgm/Engines.h"
#include "smtk/cgm/TDUUID.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

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

#include "smtk/cgm/ReadOperator_xml.h"

namespace cgmsmtk {
  namespace cgm {

// local helper
static bool hasEnding(const std::string& fullString, const std::string& ending)
{
  if (fullString.length() >= ending.length())
    return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
  else
    return false;
}

bool ReadOperator::ableToOperate()
{
  return
    this->ensureSpecification() &&
    this->specification()->isValid();
}

smtk::model::OperatorResult ReadOperator::operateInternal()
{
  smtk::attribute::FileItem::Ptr filenameItem =
    this->specification()->findFile("filename");
  smtk::attribute::StringItem::Ptr filetypeItem =
    this->specification()->findString("filetype");

  std::string filename = filenameItem->value();
  std::string filetype = filetypeItem->value();

  cgmsmtk::cgm::CAUUID::registerWithAttributeManager();
  std::string engine = "OCC";
  if (filetype.empty())
    { // Try to infer file type
    if (hasEnding(filename,".sat")) filetype = "ACIS_SAT";
    else if (hasEnding(filename,".brep")) filetype = "OCC";
    }
  if (filetype == "FACET_TYPE") engine = "FACET";
  else if (filetype == "ACIS_SAT") engine = "ACIS";
  if (!Engines::setDefault(engine))
    {
    std::cerr << "Could not set default engine to \"" << engine << "\"\n";
    return this->createResult(smtk::model::OPERATION_FAILED);
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
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::ModelEntityItem::Ptr resultModels =
    result->findModelEntity("model");

  Bridge* bridge = this->cgmBridge();
  std::string modelName = filename.substr(0, filename.find_last_of("."));
  int ne = static_cast<int>(imported.size());
  resultModels->setNumberOfValues(ne);
  for (int i = 0; i < ne; ++i)
    {
    RefEntity* entry = imported.get_and_step();
    cgmsmtk::cgm::TDUUID* refId = cgmsmtk::cgm::TDUUID::ofEntity(entry, true);
    smtk::util::UUID entId = refId->entityId();
    smtk::model::Cursor smtkEntry(this->manager(), entId);
    if (bridge->transcribe(smtkEntry, smtk::model::BRIDGE_EVERYTHING, false))
      resultModels->setValue(i, smtkEntry);
    }
  imported.reset();

  return result;
}

  } // namespace cgm
} // namespace cgmsmtk

smtkImplementsModelOperator(
  cgmsmtk::cgm::ReadOperator,
  cgm_read,
  "read",
  ReadOperator_xml,
  cgmsmtk::cgm::Bridge);
