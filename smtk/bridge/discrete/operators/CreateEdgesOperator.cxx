//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "CreateEdgesOperator.h"

#include "smtk/bridge/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"

#include "vtkModelItem.h"
#include "vtkModel.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"

#include "CreateEdgesOperator_xml.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {

  namespace discrete {

CreateEdgesOperator::CreateEdgesOperator()
{
}

bool CreateEdgesOperator::ableToOperate()
{
  smtk::model::Model model;
  return
    // The SMTK model must be valid
    (model = this->specification()->findModelEntity("model")->value().as<smtk::model::Model>()).isValid() &&
    // The CMB model must exist:
    this->discreteSession()->findModelEntity(model.entity())
    ;
}

OperatorResult CreateEdgesOperator::operateInternal()
{
  Session* opsession = this->discreteSession();

  smtk::model::EntityRef inModel =
    this->specification()->findModelEntity("model")->value();

  vtkDiscreteModelWrapper* modelWrapper =
    opsession->findModelEntity(inModel.entity());
  if (!modelWrapper)
    {
    return this->createResult(OPERATION_FAILED);
    }

  this->m_op->SetShowEdges(1);
  this->m_op->Operate(modelWrapper);
  bool ok = this->m_op->GetOperateSucceeded();

  if(ok)
    {
// TODO:
  // There seems to be a bug that will cause a crash during Sessions'
  // transcribing of the full 3D model with newly created edges and vertexes.
  // This needs to be investigated further.
/*
    smtk::model::ManagerPtr store = this->manager();
    vtkDiscreteModel* dmod = modelWrapper->GetModel();
    // Add or obtain the model's UUID
    // smtk::common::UUID mid = opsession->findOrSetEntityUUID(dmod);
    store->eraseModel(inModel.as<smtk::model::Model>());
    smtk::common::UUID mid = opsession->findOrSetEntityUUID(dmod);
    smtk::model::EntityRef c = opsession->addCMBEntityToManager(mid, dmod, store, 8);
//    store->insertModel(c.entity());
*/
    }

  OperatorResult result =
    this->createResult(
      ok ?  OPERATION_SUCCEEDED : OPERATION_FAILED);

  // TODO: Read list of new Edges created and
  //       use the session to translate them and store
  //       them in the OperatorResult (well, a subclass).

  this->addEntityToResult(result, inModel, MODIFIED);

  return result;
}

Session* CreateEdgesOperator::discreteSession() const
{
  return dynamic_cast<Session*>(this->session());
}

    } // namespace discrete
  } // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(
  SMTKDISCRETESESSION_EXPORT,
  smtk::bridge::discrete::CreateEdgesOperator,
  discrete_create_edges,
  "create edges",
  CreateEdgesOperator_xml,
  smtk::bridge::discrete::Session);
