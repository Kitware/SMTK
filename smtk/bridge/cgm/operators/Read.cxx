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

#include "smtk/bridge/cgm/Session.h"
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
  if (filetype.empty())
    { // Try to infer file type
    if (hasEnding(filename, "facet")) filetype = "FACET"; // We just want something not in an #if-clause
    // Be sure these match Session_json.h:
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

  this->addEntitiesToResult(imported, result, CREATED);

  // Set name and url property on each top-level output
  smtk::attribute::ModelEntityItem::Ptr resultModels =
    result->findModelEntity("created");
  std::string modelName = filename.substr(0, filename.find_last_of("."));
  std::size_t prefix = modelName.find_last_of("/");
  if (prefix != std::string::npos)
    modelName = modelName.substr(prefix + 1);
  std::size_t ne = resultModels->numberOfValues();
  for (std::size_t i = 0; i < ne; ++i)
    {
    std::ostringstream curModelName;
    curModelName << (modelName.empty() ? "Model" : modelName);
    if (i > 0)
      curModelName << " " << (i+1);
    smtk::model::EntityRef ent = resultModels->value(i);
    ent.setName(curModelName.str());
    ent.setStringProperty("url", filename);
    }

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
  smtk::bridge::cgm::Session);
