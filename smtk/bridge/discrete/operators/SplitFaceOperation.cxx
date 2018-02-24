//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "SplitFaceOperation.h"

#include "smtk/bridge/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/model/Events.h"
#include "smtk/model/Face.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Volume.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModel.h"
#include "vtkModelFace.h"
#include "vtkModelRegion.h"

#include <set>

#include "SplitFaceOperation_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
{

namespace discrete
{

SplitFaceOperation::SplitFaceOperation()
{
}

bool SplitFaceOperation::ableToOperate()
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

  // The CMB face to split must be valid
  if (this->fetchCMBFaceId(resource) < 0)
  {
    return false;
  }

  return true;
}

SplitFaceOperation::Result SplitFaceOperation::operateInternal()
{
  smtk::model::Model model =
    this->parameters()->findModelEntity("model")->value().as<smtk::model::Model>();

  smtk::bridge::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::bridge::discrete::Resource>(model.component()->resource());

  SessionPtr opsession = resource->discreteSession();

  vtkDiscreteModelWrapper* modelWrapper = opsession->findModelEntity(model.entity());

  // Translate SMTK inputs into CMB inputs
  this->m_op->SetFeatureAngle(this->parameters()->findDouble("feature angle")->value());

  bool ok = false;
  std::map<smtk::common::UUID, smtk::common::UUIDs> splitfacemaps;
  int totNewFaces = 0;
  smtk::common::UUID inFaceUID, faceUUID;

  // Translate SMTK inputs into CMB inputs
  smtk::attribute::ModelEntityItemPtr sourceItem =
    this->parameters()->findModelEntity("face to split");
  for (std::size_t idx = 0; idx < sourceItem->numberOfValues(); idx++)
  {
    int srcid = this->fetchCMBCellId(resource, sourceItem, static_cast<int>(idx));
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

  Result result = this->createResult(ok ? smtk::operation::Operation::Outcome::SUCCEEDED
                                        : smtk::operation::Operation::Outcome::FAILED);

  if (ok)
  {

    // Return the list of entities that were split, "modified" item in result,
    // so that remote sessions can track what records
    // need to be re-fetched.
    // Adding new faces to the "created" item, as a convenient method
    // to get newly created faces from result.
    smtk::common::UUID modelid = opsession->findOrSetEntityUUID(modelWrapper->GetModel());
    smtk::model::Model inModel(resource, modelid);
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

      modEnts.push_back(smtk::model::EntityRef(resource, inFaceUID));
      for (nit = it->second.begin(); nit != it->second.end(); ++nit)
      {
        vtkModelFace* newFace = vtkModelFace::SafeDownCast(opsession->entityForUUID(*nit));
        faceUUID = opsession->findOrSetEntityUUID(newFace);
        newEnts.push_back(smtk::model::EntityRef(resource, faceUUID));
      }
    }

    // Return the created and/or modified faces.
    if (newEnts.size() > 0)
    {
      smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
      for (auto c : newEnts)
      {
        created->appendValue(c.component());
      }
    }
    if (modEnts.size() > 0)
    {
      smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");
      for (auto m : modEnts)
      {
        modified->appendValue(m.component());
      }
    }
  }

  return result;
}

int SplitFaceOperation::fetchCMBFaceId(smtk::bridge::discrete::Resource::Ptr& resource) const
{
  vtkModelItem* item =
    resource->discreteSession()->entityForUUID(const_cast<SplitFaceOperation*>(this)
                                                 ->parameters()
                                                 ->findModelEntity("face to split")
                                                 ->value()
                                                 .entity());
  vtkModelEntity* face = dynamic_cast<vtkModelEntity*>(item);
  if (face)
    return face->GetUniquePersistentId();

  return -1;
}

int SplitFaceOperation::fetchCMBCellId(smtk::bridge::discrete::Resource::Ptr& resource,
  const smtk::attribute::ModelEntityItemPtr& entItem, int idx) const
{
  vtkModelItem* item = resource->discreteSession()->entityForUUID(entItem->value(idx).entity());

  vtkModelEntity* cell = dynamic_cast<vtkModelEntity*>(item);
  if (cell)
    return cell->GetUniquePersistentId();

  return -1;
}

const char* SplitFaceOperation::xmlDescription() const
{
  return SplitFaceOperation_xml;
}

} // namespace discrete
} // namespace bridge
} // namespace smtk
