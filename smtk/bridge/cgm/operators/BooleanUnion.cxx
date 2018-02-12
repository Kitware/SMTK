//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/operators/BooleanUnion.h"

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

#include "smtk/bridge/cgm/BooleanUnion_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
{
namespace cgm
{

smtk::operation::NewOpResult BooleanUnion::operateInternal()
{
  // The union operator preserves the first input body even when
  // keepInputs is false, so be careful not to expunge it (by
  // setting keepInputs to -1 when it would otherwise be 0).
  int keepInputs = this->findInt("keep inputs")->value();

  Models::iterator it;
  DLIList<Body*> cgmBodiesIn;
  DLIList<Body*> cgmBodiesOut;
  EntityRefArray expunged;
  bool ok = true;
  ok &= this->cgmEntities(
    *this->specification()->associations().get(), cgmBodiesIn, keepInputs, expunged);

  if (cgmBodiesIn.size() < 2)
  {
    smtkInfoMacro(log(), "Need multiple bodies to union, given " << cgmBodiesIn.size() << ".");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  DLIList<RefEntity*> imported;
  CubitStatus s = GeometryModifyTool::instance()->unite(cgmBodiesIn, cgmBodiesOut, keepInputs);
  if (s != CUBIT_SUCCESS)
  {
    smtkInfoMacro(log(), "Failed to perform union (status " << s << ").");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  smtk::operation::NewOpResult result =
    this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);

  this->addEntitiesToResult(cgmBodiesOut, result, MODIFIED);
  result->findModelEntity("expunged")->setValues(expunged.begin(), expunged.end());

  return result;
}

} // namespace cgm
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKCGMSESSION_EXPORT, smtk::bridge::cgm::BooleanUnion,
  cgm_boolean_union, "union", BooleanUnion_xml, smtk::bridge::cgm::Session);
