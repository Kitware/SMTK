//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "SplitFaceOperator.h"

#include "smtk/bridge/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/model/Events.h"
#include "smtk/model/Face.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Volume.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModel.h"
#include "vtkModelFace.h"
#include "vtkModelRegion.h"

#include <set>

#include "SplitFaceOperator_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
{

namespace discrete
{

SplitFaceOperator::SplitFaceOperator()
{
}

bool SplitFaceOperator::ableToOperate()
{
  smtk::model::Model model;
  return
    // The SMTK model must be valid
    (model = this->specification()->findModelEntity("model")->value().as<smtk::model::Model>())
      .isValid() &&
    // The CMB model must exist:
    this->discreteSession()->findModelEntity(model.entity()) &&
    // The CMB face to split must be valid
    this->fetchCMBFaceId() >= 0;
}

OperatorResult SplitFaceOperator::operateInternal()
{
  Session* opsession = this->discreteSession();

  // Translate SMTK inputs into CMB inputs
  this->m_op->SetFeatureAngle(this->specification()->findDouble("feature angle")->value());
  vtkDiscreteModelWrapper* modelWrapper =
    opsession->findModelEntity(this->specification()->findModelEntity("model")->value().entity());
  smtk::model::ManagerPtr store = this->manager();

  bool ok = false;
  std::map<smtk::common::UUID, smtk::common::UUIDs> splitfacemaps;
  int totNewFaces = 0;
  smtk::common::UUID inFaceUID, faceUUID;

  // Translate SMTK inputs into CMB inputs
  smtk::attribute::ModelEntityItemPtr sourceItem =
    this->specification()->findModelEntity("face to split");
  for (std::size_t idx = 0; idx < sourceItem->numberOfValues(); idx++)
  {
    int srcid = this->fetchCMBCellId(sourceItem, static_cast<int>(idx));
    if (srcid >= 0)
    {
      this->m_op->SetId(srcid); // "face to split"
      this->m_op->Operate(modelWrapper);
      ok = this->m_op->GetOperateSucceeded() != 0;
      if (ok)
      {
        smtk::model::EntityRef inFace = sourceItem->value(idx);
        inFaceUID = inFace.entity();

        vtkIdTypeArray* newFaceIds = this->m_op->GetCreatedModelFaceIDs();
        vtkIdType* idBuffer = reinterpret_cast<vtkIdType*>(newFaceIds->GetVoidPointer(0));
        vtkIdType length = newFaceIds->GetNumberOfComponents() * newFaceIds->GetNumberOfTuples();
        totNewFaces += length;
        for (vtkIdType tId = 0; tId < length; ++tId, ++idBuffer)
        {
          vtkIdType faceId = *idBuffer;
          vtkModelFace* face =
            dynamic_cast<vtkModelFace*>(modelWrapper->GetModelEntity(vtkModelFaceType, faceId));
          faceUUID = opsession->findOrSetEntityUUID(face);
          splitfacemaps[inFaceUID].insert(faceUUID);
        }
      }
    }
  }

  OperatorResult result = this->createResult(ok ? OPERATION_SUCCEEDED : OPERATION_FAILED);

  if (ok)
  {

    // Return the list of entities that were split, "modified" item in result,
    // so that remote sessions can track what records
    // need to be re-fetched.
    // Adding new faces to the "created" item, as a convenient method
    // to get newly created faces from result.
    smtk::common::UUID modelid = opsession->findOrSetEntityUUID(modelWrapper->GetModel());
    smtk::model::Model inModel(store, modelid);
    // this will remove and re-add the model so that the model topology and all
    // relationships will be reset properly.
    opsession->retranscribeModel(inModel);

    smtk::model::EntityRefArray modEnts;
    smtk::model::EntityRefArray newEnts;
    std::map<smtk::common::UUID, smtk::common::UUIDs>::const_iterator it;
    smtk::common::UUIDs::const_iterator nit;
    for (it = splitfacemaps.begin(); it != splitfacemaps.end(); ++it)
    {
      vtkModelFace* origFace = vtkModelFace::SafeDownCast(opsession->entityForUUID(it->first));
      inFaceUID = opsession->findOrSetEntityUUID(origFace);

      modEnts.push_back(smtk::model::EntityRef(store, inFaceUID));
      for (nit = it->second.begin(); nit != it->second.end(); ++nit)
      {
        vtkModelFace* newFace = vtkModelFace::SafeDownCast(opsession->entityForUUID(*nit));
        faceUUID = opsession->findOrSetEntityUUID(newFace);
        newEnts.push_back(smtk::model::EntityRef(store, faceUUID));
      }
    }

    // Return the created and/or modified faces.
    if (newEnts.size() > 0)
      this->addEntitiesToResult(result, newEnts, CREATED);
    if (modEnts.size() > 0)
      this->addEntitiesToResult(result, modEnts, MODIFIED);
  }

  return result;
}

Session* SplitFaceOperator::discreteSession() const
{
  return dynamic_cast<Session*>(this->session());
}

int SplitFaceOperator::fetchCMBFaceId() const
{
  vtkModelItem* item = this->discreteSession()->entityForUUID(
    this->specification()->findModelEntity("face to split")->value().entity());
  vtkModelEntity* face = dynamic_cast<vtkModelEntity*>(item);
  if (face)
    return face->GetUniquePersistentId();

  return -1;
}

int SplitFaceOperator::fetchCMBCellId(
  const smtk::attribute::ModelEntityItemPtr& entItem, int idx) const
{
  vtkModelItem* item = this->discreteSession()->entityForUUID(entItem->value(idx).entity());

  vtkModelEntity* cell = dynamic_cast<vtkModelEntity*>(item);
  if (cell)
    return cell->GetUniquePersistentId();

  return -1;
}

} // namespace discrete
} // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(SMTKDISCRETESESSION_EXPORT, smtk::bridge::discrete::SplitFaceOperator,
  discrete_split_face, "split face", SplitFaceOperator_xml, smtk::bridge::discrete::Session);
