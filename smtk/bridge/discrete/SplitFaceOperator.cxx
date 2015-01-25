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

#include "smtk/bridge/discrete/Bridge.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/model/Operator.h"
#include "smtk/model/Events.h"
#include "smtk/model/Face.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Volume.h"

#include "vtkDiscreteModelWrapper.h"
#include "vtkModel.h"
#include "vtkModelFace.h"
#include "vtkModelRegion.h"
#include "vtkModelEntity.h"

#include "SplitFaceOperator_xml.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {

  namespace discrete {

SplitFaceOperator::SplitFaceOperator()
{
}

bool SplitFaceOperator::ableToOperate()
{
  smtk::model::ModelEntity model;
  return
    // The SMTK model must be valid
    (model = this->specification()->findModelEntity("model")->value().as<smtk::model::ModelEntity>()).isValid() &&
    // The CMB model must exist:
    this->discreteBridge()->findModel(model.entity()) &&
    // The CMB face to split must be valid
    this->fetchCMBFaceId() >= 0
    ;
}

/// The current discrete bridge translation from vtkDiscreteModel to smtk model
/// does not have all the relationship set up properly, mostly the shell/face use related.
/// Therefore we have to add the raw relationship here.
static void internal_addRawRelationship(
  smtk::model::Face& face,
  smtk::model::Volume& vol1,
  smtk::model::Volume& vol2)
{
  if(!face.isValid())
    return;

  if(vol1.isValid())
    {
    face.addRawRelation(vol1);
    vol1.addRawRelation(face);
    }
  if(vol2.isValid())
    {
    face.addRawRelation(vol2);
    vol2.addRawRelation(face);
    }
}

OperatorResult SplitFaceOperator::operateInternal()
{
  Bridge* bridge = this->discreteBridge();

  // Translate SMTK inputs into CMB inputs
  this->m_op->SetFeatureAngle(
    this->specification()->findDouble("feature angle")->value());

  this->m_op->SetId(this->fetchCMBFaceId()); // "face to split"

  vtkDiscreteModelWrapper* modelWrapper =
    bridge->findModel(
      this->specification()->findModelEntity("model")->value().entity());

  this->m_op->Operate(modelWrapper);
  bool ok = this->m_op->GetOperateSucceeded();
  OperatorResult result =
    this->createResult(
      ok ?  OPERATION_SUCCEEDED : OPERATION_FAILED);

  if (ok)
    {
    // TODO: Read list of new Faces created by split and
    //       use the bridge to translate them and store
    //       them in the OperatorResult (well, a subclass).
    smtk::model::ManagerPtr store = this->manager();

    smtk::common::UUID faceUUID =
      this->specification()->findModelEntity("face to split")->value().entity();
    vtkModelFace* origFace = vtkModelFace::SafeDownCast(
      bridge->entityForUUID(faceUUID));

    vtkModelRegion* v1 = origFace->GetModelRegion(0);
    vtkModelRegion* v2 = origFace->GetModelRegion(1);
    Volume vol1 = v1 ? Volume(store, bridge->findOrSetEntityUUID(v1)) : Volume();
    Volume vol2 = v2 ? Volume(store, bridge->findOrSetEntityUUID(v2)) : Volume();

    // First, get rid of the old face that we split.
    store->erase(faceUUID);

    // Now re-add it (it will have new edges)
    faceUUID = bridge->findOrSetEntityUUID(origFace);
    smtk::model::Face c = bridge->addFaceToManager(faceUUID,
      origFace, store, true);

    internal_addRawRelationship(c, vol1, vol2);

    // Return the list of entities that were created
    // so that remote bridges can track what records
    // need to be re-fetched.
    smtk::attribute::ModelEntityItem::Ptr resultEntities =
      result->findModelEntity("entities");
    resultEntities->setNumberOfValues(1);
    resultEntities->setValue(0, c);

    smtk::attribute::IntItem::Ptr eventEntity =
      result->findInt("eventtype");
    eventEntity->setNumberOfValues(1);
    eventEntity->setValue(0, TESSELLATION_ENTRY);

    vtkIdTypeArray* newFaceIds = this->m_op->GetCreatedModelFaceIDs();
    smtk::attribute::ModelEntityItem::Ptr newEntities =
      result->findModelEntity("new entities");
    newEntities->setNumberOfValues(newFaceIds->GetMaxId() + 1);

    for (vtkIdType i = 0; i <= newFaceIds->GetMaxId(); ++i)
      {
      vtkIdType faceId = newFaceIds->GetValue(i);
      vtkModelFace* face = dynamic_cast<vtkModelFace*>(
        modelWrapper->GetModelEntity(vtkModelFaceType, faceId));
      faceUUID = bridge->findOrSetEntityUUID(face);
      smtk::model::Face cFace = bridge->addFaceToManager(faceUUID, face, store, true);
      newEntities->setValue(i, cFace);
      internal_addRawRelationship(cFace, vol1, vol2);
      /*
      this->m_op->SetCurrentNewFaceId(faceId);
      vtkIdTypeArray* spvrt = this->m_op->GetSplitEdgeVertIds();
      vtkIdTypeArray* nwvrt = this->m_op->GetCreatedModelEdgeVertIDs();
      vtkIdTypeArray* loops = this->m_op->GetFaceEdgeLoopIDs();
      */
      }
    }

  return result;
}

Bridge* SplitFaceOperator::discreteBridge() const
{
  return dynamic_cast<Bridge*>(this->bridge());
}

int SplitFaceOperator::fetchCMBFaceId() const
{
  vtkModelItem* item =
    this->discreteBridge()->entityForUUID(
      this->specification()->findModelEntity(
        "face to split")->value().entity());
  vtkModelEntity* face = dynamic_cast<vtkModelEntity*>(item);
  if (face)
    return face->GetUniquePersistentId();

  return -1;
}

    } // namespace discrete
  } // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::discrete::SplitFaceOperator,
  discrete_split_face,
  "split face",
  SplitFaceOperator_xml,
  smtk::bridge::discrete::Bridge);
