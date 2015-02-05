//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/BooleanSubtraction.h"

#include "smtk/bridge/cgm/Session.h"
#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

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

#include "smtk/bridge/cgm/BooleanSubtraction_xml.h"

using namespace smtk::model;
using smtk::attribute::ModelEntityItemPtr;

namespace smtk {
  namespace bridge {
    namespace cgm {

smtk::model::OperatorResult BooleanSubtraction::operateInternal()
{
  int keepInputs = this->findInt("keep inputs")->value();
  int imprint = this->findInt("imprint workpieces")->value();
  ModelEntityItemPtr toolsIn = this->findModelEntity("tools");
  Models bodiesIn = this->associatedEntitiesAs<Models>();

  Models::iterator it;
  DLIList<Body*> cgmBodiesIn;
  DLIList<Body*> cgmBodiesOut;
  DLIList<Body*> cgmToolsIn;
  Body* cgmBody;
  EntityRefArray expunged;
  for (it = bodiesIn.begin(); it != bodiesIn.end(); ++it)
    {
    cgmBody = dynamic_cast<Body*>(this->cgmEntity(*it));
    if (cgmBody)
      cgmBodiesIn.append(cgmBody);
    if (!keepInputs)
      {
      this->manager()->eraseModel(*it);
      expunged.push_back(*it);
      }
    }

  if (cgmBodiesIn.size() < 1)
    {
    smtkInfoMacro(log(), "Need at least 1 workpiece, none of the associated entities were valid.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::EntityRefArray::const_iterator toolIt;
  for (toolIt = toolsIn->begin(); toolIt != toolsIn->end(); ++toolIt)
    {
    cgmBody = dynamic_cast<Body*>(this->cgmEntity(*toolIt));
    if (cgmBody)
      cgmToolsIn.append(cgmBody);
    if (!keepInputs)
      {
      this->manager()->eraseModel(*toolIt);
      expunged.push_back(*it);
      }
    }

  DLIList<RefEntity*> imported;
  CubitStatus s = GeometryModifyTool::instance()->subtract(
    cgmToolsIn, cgmBodiesIn, cgmBodiesOut, imprint, keepInputs);
  if (s != CUBIT_SUCCESS)
    {
    smtkInfoMacro(log(), "Failed to perform subtraction (status " << s << ").");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  smtk::attribute::ModelEntityItem::Ptr resultBodies =
    result->findModelEntity("entities");

  Session* session = this->cgmSession();
  int numBodiesOut = cgmBodiesOut.size();
  resultBodies->setNumberOfValues(numBodiesOut);

  for (int i = 0; i < numBodiesOut; ++i)
    {
    cgmBody = cgmBodiesOut.get_and_step();
    if (!cgmBody)
      continue;

    smtk::bridge::cgm::TDUUID* refId = smtk::bridge::cgm::TDUUID::ofEntity(cgmBody, true);
    smtk::common::UUID entId = refId->entityId();
    smtk::model::EntityRef smtkEntry(this->manager(), entId);
    if (session->transcribe(smtkEntry, smtk::model::SESSION_EVERYTHING, false))
      resultBodies->setValue(i, smtkEntry);
    }

  result->findModelEntity("expunged")->setValues(expunged.begin(), expunged.end());

  return result;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::cgm::BooleanSubtraction,
  cgm_boolean_subtraction,
  "subtraction",
  BooleanSubtraction_xml,
  smtk::bridge::cgm::Session);
