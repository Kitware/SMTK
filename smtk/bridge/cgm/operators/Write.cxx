//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/Write.h"

#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/Session.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

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

#include "smtk/bridge/cgm/Write_xml.h"

namespace smtk
{
namespace bridge
{
namespace cgm
{

// local helper
static bool hasEnding(const std::string& fullString, const std::string& ending)
{
  if (fullString.length() >= ending.length())
    return (
      0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
  else
    return false;
}

smtk::model::OperatorResult Write::operateInternal()
{
  smtk::attribute::FileItem::Ptr filenameItem = this->specification()->findFile("filename");
  smtk::attribute::StringItem::Ptr filetypeItem = this->specification()->findString("filetype");
  smtk::model::Models bodies =
    this->specification()->associatedModelEntities<smtk::model::Models>();

  std::string filename = filenameItem->value();
  std::string filetype = filetypeItem->value();

  if (filetype.empty())
  { // Try to infer file type
    if (hasEnding(filename, "facet"))
      filetype = "FACET"; // We just want something not in an #if-clause
                          // Be sure these match Session_json.h:
#if defined(HAVE_ACIS)
    else if (hasEnding(filename, ".sat"))
      filetype = "ACIS_SAT";
    else if (hasEnding(filename, ".sab"))
      filetype = "ACIS_SAB";
#endif
#if defined(HAVE_OCC)
    else if (hasEnding(filename, ".brep"))
      filetype = "OCC";
#endif
#if defined(HAVE_ACIS) || (defined(HAVE_OCC) && defined(HAVE_OCC_IGES))
    else if (hasEnding(filename, ".iges"))
      filetype = "IGES";
    else if (hasEnding(filename, ".igs"))
      filetype = "IGES";
#endif
#if defined(HAVE_OCC) && defined(HAVE_OCC_STEP)
    else if (hasEnding(filename, ".step"))
      filetype = "STEP";
    else if (hasEnding(filename, ".stp"))
      filetype = "STEP";
#endif
#if defined(HAVE_OCC) && defined(HAVE_OCC_STL)
    else if (hasEnding(filename, ".stl"))
      filetype = "STL";
#endif
  }

  DLIList<RefEntity*> entities;
  RefEntity* refEntity;
  smtk::model::Models::iterator it;
  for (it = bodies.begin(); it != bodies.end(); ++it)
  {
    refEntity = this->cgmEntity(*it);
    if (refEntity)
    {
      entities.append(refEntity);
    }
    else
    {
      smtkWarningMacro(log(), "No CGM entity for SMTK model \"" << it->name() << "\" ("
                                                                << it->flagSummary() << ")");
    }
  }
  CubitStatus s;
  int numExported;
  CubitString cubitVersion;
  s = CubitCompat_export_solid_model(entities, filename.c_str(), filetype.c_str(), numExported,
    cubitVersion,
    /*logfile_name*/ NULL);
  if (s != CUBIT_SUCCESS)
  {
    smtkWarningMacro(this->manager()->log(), "Failed to save CGM model, status " << s);
    return this->createResult(smtk::model::OPERATION_FAILED);
  }

  smtk::model::OperatorResult result = this->createResult(smtk::model::OPERATION_SUCCEEDED);

  return result;
}

} // namespace cgm
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKCGMSESSION_EXPORT, smtk::bridge::cgm::Write, cgm_write, "write",
  Write_xml, smtk::bridge::cgm::Session);
