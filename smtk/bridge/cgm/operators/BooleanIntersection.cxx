//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/BooleanIntersection.h"

#include "smtk/bridge/cgm/Bridge.h"
#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ModelEntity.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
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
#include "Body.hpp"

#include "smtk/bridge/cgm/BooleanIntersection_xml.h"

using namespace smtk::model;
using smtk::attribute::ModelEntityItemPtr;

namespace smtk {
  namespace bridge {
    namespace cgm {

/// Verify that at least two input bodies have been specified.
bool BooleanIntersection::ableToOperate()
{
  this->ensureSpecification();
  bool result = true;
  std::size_t numWorkpieces = this->associatedEntitiesAs<ModelEntities>().size();
  std::size_t numTools = this->findModelEntity("tool")->numberOfValues();
  if (numWorkpieces + numTools < 2)
    {
    result = false;
    smtkInfoMacro(
      log(),
      "Need multiple bodies to intersect, given "
      << numWorkpieces << " workpieces and "
      << numTools << " tools.");
    }
  return result;
}

smtk::model::OperatorResult BooleanIntersection::operateInternal()
{
  int keepInputs = this->findInt("keep inputs")->value();
  ModelEntities bodiesIn = this->associatedEntitiesAs<ModelEntities>();
  ModelEntityItemPtr toolIn = this->findModelEntity("tool");
  Body* cgmToolBody = NULL;

  ModelEntities::iterator it;
  DLIList<Body*> cgmBodiesIn;
  DLIList<Body*> cgmBodiesOut;
  Body* cgmBody;
  for (it = bodiesIn.begin(); it != bodiesIn.end(); ++it)
    {
    cgmBody = dynamic_cast<Body*>(this->cgmEntity(*it));
    if (cgmBody)
      cgmBodiesIn.append(cgmBody);
    if (!keepInputs)
      this->manager()->eraseModel(*it);
    }

  if (toolIn->numberOfValues() > 0)
    {
    cgmToolBody = dynamic_cast<Body*>(this->cgmEntity(toolIn->value()));
    if (!keepInputs)
      this->manager()->eraseModel(toolIn->value());
    if (!cgmToolBody)
      {
      smtkInfoMacro(
        log(),
        "Tool body specified as " << toolIn->value().name() << " (" << toolIn->value().flagSummary() << ")"
        << " but no matching CGM entity exists.");
      return this->createResult(smtk::model::OPERATION_FAILED);
      }
    }

  CubitStatus s;
  DLIList<RefEntity*> imported;
  if (cgmToolBody)
    s = GeometryModifyTool::instance()->intersect(cgmToolBody, cgmBodiesIn, cgmBodiesOut, keepInputs);
  else
    s = GeometryModifyTool::instance()->intersect(cgmBodiesIn, cgmBodiesOut, keepInputs);
  if (s != CUBIT_SUCCESS)
    {
    smtkInfoMacro(log(), "Failed to perform intersection (status " << s << ").");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::ModelEntityItem::Ptr resultBodies =
    result->findModelEntity("entities");

  Bridge* bridge = this->cgmBridge();
  int numBodiesOut = cgmBodiesOut.size();
  resultBodies->setNumberOfValues(numBodiesOut);

  for (int i = 0; i < numBodiesOut; ++i)
    {
    cgmBody = cgmBodiesOut.get_and_step();
    if (!cgmBody)
      continue;

    smtk::bridge::cgm::TDUUID* refId = smtk::bridge::cgm::TDUUID::ofEntity(cgmBody, true);
    smtk::common::UUID entId = refId->entityId();
    smtk::model::Cursor smtkEntry(this->manager(), entId);
    if (bridge->transcribe(smtkEntry, smtk::model::BRIDGE_EVERYTHING, false))
      resultBodies->setValue(i, smtkEntry);
    }

  return result;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::cgm::BooleanIntersection,
  cgm_boolean_intersection,
  "intersection",
  BooleanIntersection_xml,
  smtk::bridge::cgm::Bridge);
