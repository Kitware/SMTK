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

#include "smtk/bridge/cgm/CAUUID.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/Session.h"
#include "smtk/bridge/cgm/TDUUID.h"

#include "smtk/io/Logger.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
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
  EntityRefArray expunged;
  bool ok = true;
  ok &= this->cgmEntities(*this->specification()->associations().get(), cgmBodiesIn, keepInputs, expunged);
  if (!ok)
    {
    smtkInfoMacro(log(), "Need at least 1 workpiece, none of the associated entities were valid.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  ok &= this->cgmEntities(*this->findModelEntity("tools").get(), cgmToolsIn, keepInputs, expunged);
  if (!ok)
    {
    smtkInfoMacro(log(), "Need at least 1 tool; none of the specified entities were valid.");
    return this->createResult(smtk::model::OPERATION_FAILED);
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

  this->addEntitiesToResult(cgmBodiesOut, result, MODIFIED);
  result->findModelEntity("expunged")->setValues(expunged.begin(), expunged.end());

  return result;
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  SMTKCGMSESSION_EXPORT,
  smtk::bridge::cgm::BooleanSubtraction,
  cgm_boolean_subtraction,
  "subtraction",
  BooleanSubtraction_xml,
  smtk::bridge::cgm::Session);
