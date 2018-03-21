//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "MergeOperation.h"

#include "smtk/bridge/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ResourceItem.h"

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

#include "MergeOperation_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
{

namespace discrete
{

MergeOperation::MergeOperation()
{
}

bool MergeOperation::ableToOperate()
{
  smtk::model::Model model =
    this->parameters()->findModelEntity("model")->value().as<smtk::model::Model>();

  // The SMTK model must be valid
  if (!model.isValid())
  {
    return false;
  }

  smtk::bridge::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::bridge::discrete::Resource>(model.component()->resource());

  // The CMB model must exist:
  if (!resource->discreteSession()->findModelEntity(model.entity()))
  {
    return false;
  }

  int tgtid = this->fetchCMBCellId(resource, "target cell");
  int srcid = this->fetchCMBCellId(resource, "source cell");

  // The source and target cells must be valid,
  if (srcid < 0 || tgtid < 0)
  {
    return false;
  }

  smtk::attribute::ModelEntityItemPtr sourceItem =
    this->parameters()->findModelEntity("source cell");

  // The source and target cells should not be the same
  if (sourceItem->numberOfValues() == 1 && srcid == tgtid)
  {
    return false;
  }

  smtk::model::Face tgtFace =
    this->parameters()->findModelEntity("target cell")->value().as<smtk::model::Face>();

  // Currently the discrete kernel can't merge face with edges becasue the
  // operation does not update edge relationships after face-merge
  if (!tgtFace.isValid() || tgtFace.edges().size() != 0)
  {
    return false;
  }

  return true;
}

MergeOperation::Result MergeOperation::operateInternal()
{
  smtk::model::Model model =
    this->parameters()->findModelEntity("model")->value().as<smtk::model::Model>();

  smtk::bridge::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::bridge::discrete::Resource>(model.component()->resource());

  SessionPtr opsession = resource->discreteSession();

  vtkDiscreteModelWrapper* modelWrapper = opsession->findModelEntity(model.entity());

  int tgtid = this->fetchCMBCellId(resource, "target cell");
  m_op->SetTargetId(tgtid);
  smtk::model::ManagerPtr store = std::static_pointer_cast<smtk::model::Manager>(resource);
  smtk::model::EntityRefs srcsRemoved;

  bool ok = false;
  // Translate SMTK inputs into CMB inputs
  smtk::attribute::ModelEntityItemPtr sourceItem =
    this->parameters()->findModelEntity("source cell");
  for (std::size_t idx = 0; idx < sourceItem->numberOfValues(); idx++)
  {
    int srcid = this->fetchCMBCellId(resource, sourceItem, static_cast<int>(idx));
    if (srcid >= 0 && srcid != tgtid)
    {
      m_op->SetSourceId(srcid);
      m_op->Operate(modelWrapper);
      ok = m_op->GetOperateSucceeded() != 0;
      if (ok)
      {
        smtk::model::EntityRef srcEnt = sourceItem->value(idx);
        store->erase(srcEnt.entity());
        srcsRemoved.insert(srcEnt);
      }
    }
  }

  Result result = this->createResult(ok ? smtk::operation::Operation::Outcome::SUCCEEDED
                                        : smtk::operation::Operation::Outcome::FAILED);

  if (ok)
  {
    smtk::model::EntityRef tgtEnt = this->parameters()->findModelEntity("target cell")->value();
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
    smtk::attribute::ComponentItem::Ptr modifiedEntities = result->findComponent("modified");
    modifiedEntities->appendValue(smtk::model::EntityRef(store, eid).component());

    smtk::attribute::ComponentItem::Ptr removedEntities = result->findComponent("expunged");
    removedEntities->setIsEnabled(true);
    removedEntities->setNumberOfValues(srcsRemoved.size());

    smtk::model::EntityRefs::const_iterator it;
    int rid = 0;
    for (it = srcsRemoved.begin(); it != srcsRemoved.end(); it++)
      removedEntities->setValue(rid++, it->component());
  }

  return result;
}

int MergeOperation::fetchCMBCellId(
  smtk::bridge::discrete::Resource::Ptr& resource, const std::string& pname) const
{
  vtkModelItem* item = resource->discreteSession()->entityForUUID(
    const_cast<MergeOperation*>(this)->parameters()->findModelEntity(pname)->value().entity());

  vtkModelEntity* cell = dynamic_cast<vtkModelEntity*>(item);
  if (cell)
    return cell->GetUniquePersistentId();

  return -1;
}

int MergeOperation::fetchCMBCellId(smtk::bridge::discrete::Resource::Ptr& resource,
  const smtk::attribute::ModelEntityItemPtr& entItem, int idx) const
{
  vtkModelItem* item = resource->discreteSession()->entityForUUID(entItem->value(idx).entity());

  vtkModelEntity* cell = dynamic_cast<vtkModelEntity*>(item);
  if (cell)
    return cell->GetUniquePersistentId();

  return -1;
}

const char* MergeOperation::xmlDescription() const
{
  return MergeOperation_xml;
}

} // namespace discrete
} // namespace bridge

} // namespace smtk
