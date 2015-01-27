//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "MergeOperator.h"

#include "smtk/bridge/discrete/Bridge.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/model/ModelEntity.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Volume.h"

#include "vtkModelItem.h"
#include "vtkModelEntity.h"
#include "vtkModelFace.h"
#include "vtkModelRegion.h"

#include "MergeOperator_xml.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {

  namespace discrete {

MergeOperator::MergeOperator()
{
}

bool MergeOperator::ableToOperate()
{
  smtk::model::ModelEntity model;

  return
    // The SMTK model must be valid
    (model = this->specification()->findModelEntity("model")->value().as<smtk::model::ModelEntity>()).isValid() &&
    // The CMB model must exist:
    this->discreteBridge()->findModel(model.entity()) &&
    // The source and target cells must be valid:
    this->fetchCMBCellId("source cell") >= 0 &&
    this->fetchCMBCellId("target cell") >= 0
    ;
}

OperatorResult MergeOperator::operateInternal()
{
  Bridge* bridge = this->discreteBridge();

  // Translate SMTK inputs into CMB inputs
  this->m_op->SetSourceId(this->fetchCMBCellId("source cell"));
  this->m_op->SetTargetId(this->fetchCMBCellId("target cell"));

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
    smtk::model::ManagerPtr store = this->manager();

    smtk::model::Cursor srcEnt =
      this->specification()->findModelEntity("source cell")->value();
    smtk::model::Cursor tgtEnt =
      this->specification()->findModelEntity("target cell")->value();

    // Get rid of the old entity.
/*
    Cursors bdys = srcEnt.as<CellEntity>().lowerDimensionalBoundaries(-1);
    for (Cursors::iterator bit = bdys.begin(); bit != bdys.end(); ++bit)
      {
      //std::cout << "Erasing " << bit->flagSummary(0) << " " << bit->entity() << "\n";
      store->erase(bit->entity());
      }
 */
    store->erase(srcEnt.entity());

    // re-add target
    smtk::common::UUID eid = tgtEnt.entity();
    vtkModelItem* origItem =
      bridge->entityForUUID(eid);

    store->erase(eid);

    // Now re-add it (it will have new edges)
    eid = bridge->findOrSetEntityUUID(origItem);
    smtk::model::Cursor c = bridge->addCMBEntityToManager(
      eid, origItem, store, true);
    if(vtkModelFace* origFace = vtkModelFace::SafeDownCast(origItem))
      {
      vtkModelRegion* v1 = origFace->GetModelRegion(0);
      vtkModelRegion* v2 = origFace->GetModelRegion(1);
      Volume vol1 = v1 ? Volume(store, bridge->findOrSetEntityUUID(v1)) : Volume();
      Volume vol2 = v2 ? Volume(store, bridge->findOrSetEntityUUID(v2)) : Volume();
      if(vol1.isValid())
        {
        c.addRawRelation(vol1);
        vol1.addRawRelation(c);
        }
      if(vol2.isValid())
        {
        c.addRawRelation(vol2);
        vol2.addRawRelation(c);
        }
      }

    // Return the list of entities that were created
    // so that remote bridges can track what records
    // need to be re-fetched.
    smtk::attribute::ModelEntityItem::Ptr resultEntities =
      result->findModelEntity("entities");
    resultEntities->setNumberOfValues(1);
    resultEntities->setValue(0, c);

    smtk::attribute::ModelEntityItem::Ptr removedEntities =
      result->findModelEntity("expunged");
    removedEntities->setNumberOfValues(1);
    removedEntities->setValue(0, srcEnt);

    smtk::attribute::IntItem::Ptr eventEntity =
      result->findInt("event type");
    eventEntity->setNumberOfValues(1);
    eventEntity->setValue(0, TESSELLATION_ENTRY);

    }

  return result;
}

Bridge* MergeOperator::discreteBridge() const
{
  return dynamic_cast<Bridge*>(this->bridge());
}

int MergeOperator::fetchCMBCellId(const std::string& pname) const
{
  vtkModelItem* item =
    this->discreteBridge()->entityForUUID(
      this->specification()->findModelEntity(pname)->value().entity());

  vtkModelEntity* cell = dynamic_cast<vtkModelEntity*>(item);
  if (cell)
    return cell->GetUniquePersistentId();

  return -1;
}

    } // namespace discrete
  } // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::discrete::MergeOperator,
  discrete_merge,
  "merge",
  MergeOperator_xml,
  smtk::bridge::discrete::Bridge);
