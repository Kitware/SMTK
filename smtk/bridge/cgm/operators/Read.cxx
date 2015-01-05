//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/Read.h"

#include "smtk/bridge/cgm/Bridge.h"
#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Manager.h"

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

#include "smtk/bridge/cgm/Read_xml.h"

namespace smtk {
  namespace bridge {
    namespace cgm {

// local helper
static bool hasEnding(const std::string& fullString, const std::string& ending)
{
  if (fullString.length() >= ending.length())
    return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
  else
    return false;
}

smtk::model::OperatorResult Read::operateInternal()
{
  smtk::attribute::FileItem::Ptr filenameItem =
    this->specification()->findFile("filename");
  smtk::attribute::StringItem::Ptr filetypeItem =
    this->specification()->findString("filetype");

  std::string filename = filenameItem->value();
  std::string filetype = filetypeItem->value();

  smtk::bridge::cgm::CAUUID::registerWithAttributeManager();
  std::string engine = "OCC";
  if (filetype.empty())
    { // Try to infer file type
    if (hasEnding(filename, "facet")) filetype = "FACET"; // We just want something not in an #if-clause
    // Be sure these match Bridge_json.h:
#if defined(HAVE_ACIS)
    else if (hasEnding(filename,".sat")) filetype = "ACIS_SAT";
    else if (hasEnding(filename,".sab")) filetype = "ACIS_SAB";
#endif
#if defined(HAVE_OCC)
    else if (hasEnding(filename,".brep")) filetype = "OCC";
#endif
#if defined(HAVE_ACIS) || (defined(HAVE_OCC) && defined(HAVE_OCC_IGES))
    else if (hasEnding(filename,".iges")) filetype = "IGES";
    else if (hasEnding(filename,".igs")) filetype = "IGES";
#endif
#if defined(HAVE_OCC) && defined(HAVE_OCC_STEP)
    else if (hasEnding(filename,".step")) filetype = "STEP";
    else if (hasEnding(filename,".stp")) filetype = "STEP";
#endif
#if defined(HAVE_OCC) && defined(HAVE_OCC_STL)
    else if (hasEnding(filename,".stl")) filetype = "STL";
#endif
    }
  // TODO: Both ACIS and OCC can provide IGES and STEP support (but do not always).
  //       Figure out how to choose the correct engine (or at least not an improper one).
  if (filetype == "FACET_TYPE") engine = "FACET";
  else if (filetype == "ACIS_SAT") engine = "ACIS";
  try
    {
    if (!Engines::setDefault(engine))
      {
      smtkInfoMacro(this->manager()->log(),
        "Could not set default engine to \"" << engine << "\"");
      return this->createResult(smtk::model::OPERATION_FAILED);
      }
    }
  catch (std::exception& e)
    {
    smtkInfoMacro(this->manager()->log(),
      "Exception thrown while setting default engine: \"" << e.what() << "\"");
    }

  smtkInfoMacro(this->manager()->log(),
    "Default modeler now"
    " \"" << GeometryQueryTool::instance()->get_gqe()->modeler_type() << "\"");
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
    smtkInfoMacro(this->manager()->log(),
      "Failed to import CGM model, status " << s);
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
    smtk::bridge::cgm::TDUUID* refId = smtk::bridge::cgm::TDUUID::ofEntity(entry, true);
    smtk::common::UUID entId = refId->entityId();
    smtk::model::Cursor smtkEntry(this->manager(), entId);
    if (bridge->transcribe(smtkEntry, smtk::model::BRIDGE_EVERYTHING, false))
      {
      resultModels->setValue(i, smtkEntry);
      this->manager()->setBridgeForModel(bridge->shared_from_this(), entId);
      }
    }
  imported.reset();


  return result;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::cgm::Read,
  cgm_read,
  "read",
  Read_xml,
  smtk::bridge::cgm::Bridge);
