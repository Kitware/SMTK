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

#include "smtk/bridge/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/model/Model.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Volume.h"

#include "vtkModelItem.h"
#include "vtkModel.h"
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
  smtk::model::Model model;

  return
    // The SMTK model must be valid
    (model = this->specification()->findModelEntity("model")->value().as<smtk::model::Model>()).isValid() &&
    // The CMB model must exist:
    this->discreteSession()->findModelEntity(model.entity()) &&
    // The source and target cells must be valid:
    this->fetchCMBCellId("source cell") >= 0 &&
    this->fetchCMBCellId("target cell") >= 0
    ;
}

OperatorResult MergeOperator::operateInternal()
{
  Session* opsession = this->discreteSession();

  vtkDiscreteModelWrapper* modelWrapper =
    opsession->findModelEntity(
      this->specification()->findModelEntity("model")->value().entity());
  this->m_op->SetTargetId(this->fetchCMBCellId("target cell"));
  smtk::model::ManagerPtr store = this->manager();
  smtk::model::EntityRefs srcsRemoved;

  bool ok = false;
  // Translate SMTK inputs into CMB inputs
  smtk::attribute::ModelEntityItemPtr sourceItem =
    this->specification()->findModelEntity("source cell");
  for(std::size_t idx=0; idx<sourceItem->numberOfValues(); idx++)
    {
    int srcid = this->fetchCMBCellId(sourceItem, idx);
    if(srcid >= 0)
      {
      this->m_op->SetSourceId(srcid);
      this->m_op->Operate(modelWrapper);
      ok = this->m_op->GetOperateSucceeded();
      if(ok)
        {
        smtk::model::EntityRef srcEnt = sourceItem->value(idx);
        store->erase(srcEnt.entity());
        srcsRemoved.insert(srcEnt);
        }
      }
    }

  OperatorResult result =
    this->createResult(
      ok ?  OPERATION_SUCCEEDED : OPERATION_FAILED);

  if (ok)
    {
    smtk::model::EntityRef tgtEnt =
      this->specification()->findModelEntity("target cell")->value();

    // re-add target
    smtk::common::UUID eid = tgtEnt.entity();
    vtkModelItem* origItem =
      opsession->entityForUUID(eid);

    store->erase(eid);

    // Now re-add it (it will have new edges)
    eid = opsession->findOrSetEntityUUID(origItem);
    smtk::model::EntityRef c = opsession->addCMBEntityToManager(
      eid, origItem, store, true);
    if(vtkModelFace* origFace = vtkModelFace::SafeDownCast(origItem))
      {
      vtkModelRegion* v1 = origFace->GetModelRegion(0);
      vtkModelRegion* v2 = origFace->GetModelRegion(1);
      Volume vol1 = v1 ? Volume(store, opsession->findOrSetEntityUUID(v1)) : Volume();
      Volume vol2 = v2 ? Volume(store, opsession->findOrSetEntityUUID(v2)) : Volume();
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
    // so that remote sessions can track what records
    // need to be re-fetched.
    this->addEntityToResult(result, c, MODIFIED);

    smtk::attribute::ModelEntityItem::Ptr removedEntities =
      result->findModelEntity("expunged");
    removedEntities->setIsEnabled(true);
    removedEntities->setNumberOfValues(srcsRemoved.size());
    removedEntities->setIsEnabled(true);

    smtk::model::EntityRefs::const_iterator it;
    int rid = 0;
    for (it=srcsRemoved.begin(); it != srcsRemoved.end(); it++)
      removedEntities->setValue(rid++, *it);

    }

  return result;
}

Session* MergeOperator::discreteSession() const
{
  return dynamic_cast<Session*>(this->session());
}

int MergeOperator::fetchCMBCellId(const std::string& pname) const
{
  vtkModelItem* item =
    this->discreteSession()->entityForUUID(
      this->specification()->findModelEntity(pname)->value().entity());

  vtkModelEntity* cell = dynamic_cast<vtkModelEntity*>(item);
  if (cell)
    return cell->GetUniquePersistentId();

  return -1;
}

int MergeOperator::fetchCMBCellId(
  const smtk::attribute::ModelEntityItemPtr& entItem, int idx ) const
{
  vtkModelItem* item =
    this->discreteSession()->entityForUUID(entItem->value(idx).entity());

  vtkModelEntity* cell = dynamic_cast<vtkModelEntity*>(item);
  if (cell)
    return cell->GetUniquePersistentId();

  return -1;
}

    } // namespace discrete
  } // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(
  SMTKDISCRETESESSION_EXPORT,
  smtk::bridge::discrete::MergeOperator,
  discrete_merge,
  "merge face",
  MergeOperator_xml,
  smtk::bridge::discrete::Session);
