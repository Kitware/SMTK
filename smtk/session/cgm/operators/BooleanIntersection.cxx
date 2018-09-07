//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/cgm/operators/BooleanIntersection.h"

#include "smtk/session/cgm/CAUUID.h"
#include "smtk/session/cgm/Engines.h"
#include "smtk/session/cgm/Session.h"
#include "smtk/session/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "Body.hpp"
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

#include "smtk/session/cgm/BooleanIntersection_xml.h"

using namespace smtk::model;
using smtk::attribute::ComponentItemPtr;

namespace smtk
{
namespace session
{
namespace cgm
{

/// Verify that at least two input bodies have been specified.
bool BooleanIntersection::ableToOperate()
{
  this->ensureSpecification();
  bool result = true;
  std::size_t numWorkpieces = this->associatedEntitiesAs<Models>().size();
  std::size_t numTools = this->findComponent("tool")->numberOfValues();
  if (numWorkpieces + numTools < 2)
  {
    result = false;
    smtkInfoMacro(log(), "Need multiple bodies to intersect, given "
        << numWorkpieces << " workpieces and " << numTools << " tools.");
  }
  return result;
}

smtk::operation::OperationResult BooleanIntersection::operateInternal()
{
  int keepInputs = this->findInt("keep inputs")->value();
  Models bodiesIn = this->associatedEntitiesAs<Models>();
  ComponentItemPtr toolIn = this->findComponent("tool");
  Body* cgmToolBody = NULL;

  Models::iterator it;
  DLIList<Body*> cgmBodiesIn;
  DLIList<Body*> cgmBodiesOut;
  EntityRefArray expunged;

  bool ok = true;
  ok &= this->cgmEntities(*this->specification()->associations().get(), cgmBodiesIn,
    /* keep_inputs */ 1, expunged);

  // Stop if any of the workpieces don't exist as CGM entities:
  if (!ok)
  {
    smtkInfoMacro(log(), "One or more workpiece inputs had no matching CGM entity.");
    return this->createResult(smtk::operation::Operation::OPERATION_FAILED);
  }

  if (toolIn->numberOfValues() > 0)
  {
    DLIList<Body*> cgmToolBodies;
    ok &=
      this->cgmEntities(*this->findComponent("tool").get(), cgmToolBodies, keepInputs, expunged);
    if (!ok)
    {
      smtkInfoMacro(log(), "Tool body specified as " << toolIn->value().name() << " ("
                                                     << toolIn->value().flagSummary() << ")"
                                                     << " but no matching CGM entity exists.");
      return this->createResult(smtk::operation::Operation::OPERATION_FAILED);
    }
    cgmToolBody = cgmToolBodies[0];
  }
  else if (!keepInputs)
  { // All but the first input will be expunged.
    smtk::attribute::ReferenceItem::const_iterator wit;
    auto assoc = this->specification()->associations();
    wit = assoc->begin();
    for (++wit; wit != assoc->end(); ++wit)
    {
      auto entry = std::dynamic_pointer_cast<smtk::model::Entity>(*wit);
      this->resource()->erase(entry);
      expunged.push_back(entry);
    }
  }

  CubitStatus s;
  DLIList<RefEntity*> imported;
  if (cgmToolBody)
    s =
      GeometryModifyTool::instance()->intersect(cgmToolBody, cgmBodiesIn, cgmBodiesOut, keepInputs);
  else
    s = GeometryModifyTool::instance()->intersect(cgmBodiesIn, cgmBodiesOut, keepInputs);
  if (s != CUBIT_SUCCESS)
  {
    smtkInfoMacro(log(), "Failed to perform intersection (status " << s << ").");
    return this->createResult(smtk::operation::Operation::OPERATION_FAILED);
  }

  smtk::operation::OperationResult result =
    this->createResult(smtk::operation::Operation::OPERATION_SUCCEEDED);

  this->addEntitiesToResult(cgmBodiesOut, result, MODIFIED);
  result->findComponent("expunged")->setValues(expunged.begin(), expunged.end());

  return result;
}

} // namespace cgm
} //namespace session
} // namespace smtk

smtkImplementsModelOperation(SMTKCGMSESSION_EXPORT, smtk::session::cgm::BooleanIntersection,
  cgm_boolean_intersection, "intersection", BooleanIntersection_xml, smtk::session::cgm::Session);
