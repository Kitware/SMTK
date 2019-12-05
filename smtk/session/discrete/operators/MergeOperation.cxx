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

#include "smtk/session/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Face.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Volume.h"

#include "smtk/session/discrete/kernel/Model/vtkModel.h"
#include "smtk/session/discrete/kernel/Model/vtkModelFace.h"
#include "smtk/session/discrete/kernel/Model/vtkModelItem.h"
#include "smtk/session/discrete/kernel/Model/vtkModelRegion.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"

#include "MergeOperation_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace session
{

namespace discrete
{

MergeOperation::MergeOperation()
{
}

bool MergeOperation::ableToOperate()
{
  smtk::model::Model model =
    this->parameters()->findComponent("model")->valueAs<smtk::model::Entity>();

  // The SMTK model must be valid
  if (!model.isValid())
  {
    return false;
  }

  smtk::session::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::discrete::Resource>(model.component()->resource());

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
    smtkErrorMacro(this->log(), "Source and target cells must have valid IDs.");
    return false;
  }

  auto sourceItem = this->parameters()->associations();

  // The source and target cells should not be the same
  if (sourceItem->numberOfValues() == 1 && srcid == tgtid)
  {
    smtkErrorMacro(this->log(), "Source and target cells must be different.");
    return false;
  }

  smtk::model::Face tgtFace =
    this->parameters()->findComponent("target cell")->valueAs<smtk::model::Entity>();

  // Currently the discrete kernel can't merge face with edges becasue the
  // operation does not update edge relationships after face-merge
  if (!tgtFace.isValid() || !tgtFace.edges().empty())
  {
    smtkErrorMacro(this->log(), "Merging faces with edges is not supported.");
    return false;
  }

  return true;
}

MergeOperation::Result MergeOperation::operateInternal()
{
  smtk::model::Model model =
    this->parameters()->findComponent("model")->valueAs<smtk::model::Entity>();

  smtk::session::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::discrete::Resource>(model.component()->resource());

  SessionPtr opsession = resource->discreteSession();

  vtkDiscreteModelWrapper* modelWrapper = opsession->findModelEntity(model.entity());

  int tgtid = this->fetchCMBCellId(resource, "target cell");
  m_op->SetTargetId(tgtid);
  smtk::model::ResourcePtr store = std::static_pointer_cast<smtk::model::Resource>(resource);
  smtk::model::EntityRefs srcsRemoved;

  bool ok = false;
  // Translate SMTK inputs into CMB inputs
  auto sourceItem = this->parameters()->associations();
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
        smtk::model::EntityRef srcEnt = sourceItem->valueAs<smtk::model::Entity>(idx);
        store->erase(srcEnt.entity());
        srcsRemoved.insert(srcEnt);
      }
    }
  }

  Result result = this->createResult(ok ? smtk::operation::Operation::Outcome::SUCCEEDED
                                        : smtk::operation::Operation::Outcome::FAILED);

  if (ok)
  {
    smtk::model::EntityRef tgtEnt =
      this->parameters()->findComponent("target cell")->valueAs<smtk::model::Entity>();
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
  smtk::session::discrete::Resource::Ptr& resource, const std::string& pname) const
{
  auto self = const_cast<MergeOperation*>(this);
  auto refItem = self->parameters()->findReference(pname);
  if (!refItem && (refItem = self->parameters()->associations())->name() != pname)
  {
    return -1;
  }
  vtkModelItem* item = resource->discreteSession()->entityForUUID(refItem->objectValue()->id());

  vtkModelEntity* cell = dynamic_cast<vtkModelEntity*>(item);
  if (cell)
    return cell->GetUniquePersistentId();

  return -1;
}

int MergeOperation::fetchCMBCellId(smtk::session::discrete::Resource::Ptr& resource,
  const smtk::attribute::ReferenceItemPtr& entItem, int idx) const
{
  vtkModelItem* item = resource->discreteSession()->entityForUUID(entItem->objectValue(idx)->id());

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
} // namespace session

} // namespace smtk
