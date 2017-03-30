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

#include "smtk/model/CellEntity.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Face.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Volume.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModel.h"
#include "vtkModelFace.h"
#include "vtkModelItem.h"
#include "vtkModelRegion.h"

#include "MergeOperator_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
{

namespace discrete
{

MergeOperator::MergeOperator()
{
}

bool MergeOperator::ableToOperate()
{
  smtk::model::Model model;
  int tgtid = this->fetchCMBCellId("target cell");
  int srcid = this->fetchCMBCellId("source cell");
  smtk::attribute::ModelEntityItemPtr sourceItem =
    this->specification()->findModelEntity("source cell");
  smtk::model::Face tgtFace =
    this->specification()->findModelEntity("target cell")->value().as<smtk::model::Face>();

  return
    // The SMTK model must be valid
    (model = this->specification()->findModelEntity("model")->value().as<smtk::model::Model>())
      .isValid() &&
    // The CMB model must exist:
    this->discreteSession()->findModelEntity(model.entity()) &&
    // The source and target cells must be valid,
    srcid >= 0 && tgtid >= 0 &&
    // The source and target cells should not be the same
    (sourceItem->numberOfValues() > 1 || srcid != tgtid) &&
    // Currently the discrete kernel can't merge face with edges becasue the
    // operation does not update edge relationships after face-merge
    (tgtFace.isValid() && tgtFace.edges().size() == 0);
}

OperatorResult MergeOperator::operateInternal()
{
  Session* opsession = this->discreteSession();
  smtk::model::Model model =
    this->specification()->findModelEntity("model")->value().as<smtk::model::Model>();
  vtkDiscreteModelWrapper* modelWrapper = opsession->findModelEntity(model.entity());
  int tgtid = this->fetchCMBCellId("target cell");
  this->m_op->SetTargetId(tgtid);
  smtk::model::ManagerPtr store = this->manager();
  smtk::model::EntityRefs srcsRemoved;

  bool ok = false;
  // Translate SMTK inputs into CMB inputs
  smtk::attribute::ModelEntityItemPtr sourceItem =
    this->specification()->findModelEntity("source cell");
  for (std::size_t idx = 0; idx < sourceItem->numberOfValues(); idx++)
  {
    int srcid = this->fetchCMBCellId(sourceItem, static_cast<int>(idx));
    if (srcid >= 0 && srcid != tgtid)
    {
      this->m_op->SetSourceId(srcid);
      this->m_op->Operate(modelWrapper);
      ok = this->m_op->GetOperateSucceeded() != 0;
      if (ok)
      {
        smtk::model::EntityRef srcEnt = sourceItem->value(idx);
        store->erase(srcEnt.entity());
        srcsRemoved.insert(srcEnt);
      }
    }
  }

  OperatorResult result = this->createResult(ok ? OPERATION_SUCCEEDED : OPERATION_FAILED);

  if (ok)
  {
    smtk::model::EntityRef tgtEnt = this->specification()->findModelEntity("target cell")->value();
    smtk::common::UUID eid = tgtEnt.entity();
    vtkModelItem* origItem = opsession->entityForUUID(eid);

    smtk::common::UUID modelid = opsession->findOrSetEntityUUID(modelWrapper->GetModel());
    smtk::model::Model inModel(store, modelid);
    // this will remove and re-add the model so that the model topology and all
    // relationships will be reset properly.
    opsession->retranscribeModel(inModel);

    eid = opsession->findOrSetEntityUUID(origItem);

    // Return the list of entities that were created
    // so that remote sessions can track what records
    // need to be re-fetched.
    this->addEntityToResult(result, smtk::model::EntityRef(store, eid), MODIFIED);

    smtk::attribute::ModelEntityItem::Ptr removedEntities = result->findModelEntity("expunged");
    removedEntities->setIsEnabled(true);
    removedEntities->setNumberOfValues(srcsRemoved.size());

    smtk::model::EntityRefs::const_iterator it;
    int rid = 0;
    for (it = srcsRemoved.begin(); it != srcsRemoved.end(); it++)
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
  vtkModelItem* item = this->discreteSession()->entityForUUID(
    this->specification()->findModelEntity(pname)->value().entity());

  vtkModelEntity* cell = dynamic_cast<vtkModelEntity*>(item);
  if (cell)
    return cell->GetUniquePersistentId();

  return -1;
}

int MergeOperator::fetchCMBCellId(const smtk::attribute::ModelEntityItemPtr& entItem, int idx) const
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

smtkImplementsModelOperator(SMTKDISCRETESESSION_EXPORT, smtk::bridge::discrete::MergeOperator,
  discrete_merge, "merge face", MergeOperator_xml, smtk::bridge::discrete::Session);
