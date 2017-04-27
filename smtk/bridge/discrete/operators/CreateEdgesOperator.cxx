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

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModel.h"
#include "vtkModelFace.h"
#include "vtkModelItem.h"
#include "vtkModelItemIterator.h"

#include "CreateEdgesOperator_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
{

namespace discrete
{

CreateEdgesOperator::CreateEdgesOperator()
{
}

bool CreateEdgesOperator::ableToOperate()
{
  smtk::model::Model model =
    this->specification()->findModelEntity("model")->value().as<smtk::model::Model>();
  if (!model.isValid())
  {
    return false;
  }
  vtkDiscreteModelWrapper* modelWrapper = this->discreteSession()->findModelEntity(model.entity());
  if (!modelWrapper)
  {
    return false;
  }

  bool operable = true;
  // verify that faces in the model do not have edges already.
  vtkModelItemIterator* iter = modelWrapper->GetModel()->NewIterator(vtkModelFaceType);
  for (iter->Begin(); !iter->IsAtEnd(); iter->Next())
  {
    vtkModelFace* face = vtkModelFace::SafeDownCast(iter->GetCurrentItem());
    if (face && face->GetNumberOfModelEdges() > 0)
    {
      operable = false;
      break;
    }
  }
  iter->Delete();

  return operable;
}

OperatorResult CreateEdgesOperator::operateInternal()
{
  Session* opsession = this->discreteSession();

  smtk::model::EntityRef inModel = this->specification()->findModelEntity("model")->value();

  vtkDiscreteModelWrapper* modelWrapper = opsession->findModelEntity(inModel.entity());
  if (!modelWrapper)
  {
    return this->createResult(OPERATION_FAILED);
  }

  this->m_op->SetShowEdges(1);
  this->m_op->Operate(modelWrapper);
  bool ok = this->m_op->GetOperateSucceeded() != 0;
  OperatorResult result = this->createResult(ok ? OPERATION_SUCCEEDED : OPERATION_FAILED);

  if (ok)
  {
    // this will remove and re-add the model so that the model topology and all
    // relationships will be reset properly.
    opsession->retranscribeModel(inModel);
    smtk::model::EntityRefArray modEnts;
    modEnts.push_back(inModel);

    // also mark all model faces are modified since there are likely new edges created
    smtk::common::UUID faceUID;
    vtkModelItemIterator* iter = modelWrapper->GetModel()->NewIterator(vtkModelFaceType);
    for (iter->Begin(); !iter->IsAtEnd(); iter->Next())
    {
      vtkModelFace* face = vtkModelFace::SafeDownCast(iter->GetCurrentItem());
      faceUID = opsession->findOrSetEntityUUID(face);
      modEnts.push_back(smtk::model::EntityRef(opsession->manager(), faceUID));
    }
    iter->Delete();

    result->findModelEntity("tess_changed")->setValue(inModel);
    this->addEntitiesToResult(result, modEnts, MODIFIED);
  }

  return result;
}

Session* CreateEdgesOperator::discreteSession() const
{
  return dynamic_cast<Session*>(this->session());
}

} // namespace discrete
} // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(SMTKDISCRETESESSION_EXPORT, smtk::bridge::discrete::CreateEdgesOperator,
  discrete_create_edges, "create edges", CreateEdgesOperator_xml, smtk::bridge::discrete::Session);
