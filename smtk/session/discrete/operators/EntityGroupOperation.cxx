//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "EntityGroupOperation.h"

#include "smtk/session/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"

#include "smtk/session/discrete/kernel/Model/vtkModel.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdList.h"
#include "vtkModelMaterial.h"
#include "vtkModelUserName.h"

#include "EntityGroupOperation_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace session
{

namespace discrete
{

EntityGroupOperation::EntityGroupOperation()
{
}

bool EntityGroupOperation::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  smtk::model::Model model = this->parameters()->associations()->valueAs<smtk::model::Entity>();

  if (!model.isValid())
  {
    return false;
  }

  smtk::session::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::discrete::Resource>(model.resource());

  if (!resource->discreteSession()->findModelEntity(model.entity()))
  {
    return false;
  }

  // for Create operation, we just need model, and default entity
  // type will be Face;
  // for Destroy and Modify operation, we need cell group is set
  smtk::attribute::StringItem::Ptr optypeItem = this->parameters()->findString("Operation");

  bool able2Op = false;

  std::string optype = optypeItem->value();
  if (optype == "Create") //only need model;
  {
    able2Op = true;
  }
  if (optype == "Remove")
  {
    able2Op = this->fetchCMBCell(resource, "remove cell group") != NULL;
  }
  else if (optype == "Modify")
  {
    able2Op = this->fetchCMBCell(resource, "modify cell group") != NULL &&
      (this->fetchCMBCell(resource, "cell to add") ||
                this->fetchCMBCell(resource, "cell to remove"));
  }

  return able2Op;
}

int EntityGroupOperation::createBoundaryGroup(vtkDiscreteModelWrapper* modelWrapper)
{
  int entType = modelWrapper->GetModel()->GetModelDimension() == 3
    ? vtkModelFaceType
    : (modelWrapper->GetModel()->GetModelDimension() == 2 ? vtkModelEdgeType : -1);
  if (entType == -1)
    return -1;

  m_opBoundary->SetBuildEnityType(entType);
  m_opBoundary->Build(modelWrapper);
  int grpId = m_opBoundary->GetBuiltModelEntityGroupId();
  return grpId;
}

int EntityGroupOperation::createDomainSet(vtkDiscreteModelWrapper* modelWrapper)
{
  m_opDomain->Build(modelWrapper);
  int dsId = m_opDomain->GetBuiltMaterialId();
  return dsId;
}

EntityGroupOperation::Result EntityGroupOperation::operateInternal()
{
  // ableToOperate should have verified that model is valid
  smtk::model::Model model = this->parameters()->associations()->valueAs<smtk::model::Entity>();

  smtk::session::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::discrete::Resource>(model.component()->resource());

  smtk::model::ResourcePtr pstore = std::static_pointer_cast<smtk::model::Resource>(resource);
  SessionPtr opsession = resource->discreteSession();

  vtkDiscreteModelWrapper* modelWrapper = opsession->findModelEntity(model.entity());
  bool ok = false;
  smtk::model::Group bgroup;
  smtk::model::EntityRefArray modGrps;
  smtk::model::EntityRefs grpsRemoved;
  // Translate SMTK inputs into CMB inputs
  smtk::attribute::StringItem::Ptr optypeItem = this->parameters()->findString("Operation");
  std::string optype = optypeItem->value();
  if (optype == "Create") //only need model
  {
    int gType = this->parameters()->findInt("group type")->value();
    std::string gName = this->parameters()->findString("group name")->value();

    int grpId = -1;
    if (gType == 0) // boundary group
      grpId = this->createBoundaryGroup(modelWrapper);
    else if (gType == 1) // domain set
      grpId = this->createDomainSet(modelWrapper);
    ok = grpId >= 0;

    if (ok)
    {

      BitFlags mask =
        // Boundary group, 3d => Face group; 2d => edge group.
        (gType == 0) ? (modelWrapper->GetModel()->GetModelDimension() == 3 ? smtk::model::FACE
                                                                           : smtk::model::EDGE)
                     :
                     // Domain set, 3d => volume group; 2d => face group.
        (modelWrapper->GetModel()->GetModelDimension() == 3 ? smtk::model::VOLUME
                                                            : smtk::model::FACE);
      int groupType = gType == 0 ? vtkDiscreteModelEntityGroupType : vtkModelMaterialType;
      vtkModelEntity* grp = modelWrapper->GetModelEntity(groupType, grpId);

      // modify the new group directly with "cell to add" item, if it exists
      // and has entries. This covers the usecase to create a group with entities
      ok = this->modifyGroup(resource, modelWrapper, grp, true, modGrps);
      if (ok)
      {
        if (!gName.empty())
          vtkModelUserName::SetUserName(grp, gName.c_str());
        smtk::common::UUID grpUUID = opsession->findOrSetEntityUUID(grp);

        // The group itself should be added too
        smtk::model::EntityRef grpRef = opsession->addCMBEntityToResource(grpUUID, grp, pstore, 0);
        bgroup = grpRef.as<smtk::model::Group>();
        bgroup.setMembershipMask(mask);
        bgroup.setName(gName);
        // Add group to model's relationship
        model.addGroup(bgroup);
        // do we really nee this? This new group is in "created" item in result
        //modGrps.push_back(bgroup);
        std::cout << "new group: " << bgroup.name() << " id: " << grpUUID.toString() << "\n";
      }
    }
  }
  else if (optype == "Remove")
  {
    auto remgrpItem = this->parameters()->findComponent("remove cell group");
    for (std::size_t idx = 0; idx < remgrpItem->numberOfValues(); idx++)
    {
      vtkModelEntity* modEntity = this->fetchCMBCell(resource, remgrpItem, static_cast<int>(idx));
      if (!modEntity)
        continue;
      if (modEntity->GetType() == vtkModelMaterialType)
      {
        m_opDomain->SetId(modEntity->GetUniquePersistentId());
        m_opDomain->Destroy(modelWrapper);
        ok = m_opDomain->GetDestroySucceeded() != 0;
      }
      else if (modEntity->GetType() == vtkDiscreteModelEntityGroupType)
      {
        m_opBoundary->SetId(modEntity->GetUniquePersistentId());
        m_opBoundary->Destroy(modelWrapper);
        ok = m_opBoundary->GetDestroySucceeded() != 0;
      }
      if (ok)
      {
        // get rid of the group from resource
        smtk::model::EntityRef grpRem = remgrpItem->valueAs<smtk::model::Entity>(idx);
        model.removeGroup(grpRem.as<smtk::model::Group>());
        pstore->erase(grpRem);
        std::cout << "Removed " << grpRem.name() << " to " << model.name() << "\n";
        grpsRemoved.insert(grpRem);
      }
    }
  }
  else if (optype == "Modify")
  {
    vtkModelEntity* grpEntity = this->fetchCMBCell(resource, "modify cell group");
    ok = this->modifyGroup(resource, modelWrapper, grpEntity, false, modGrps);

    if (ok)
    {
      // get rid of the group from resource
      smtk::model::EntityRef grpC =
        this->parameters()->findComponent("modify cell group")->valueAs<smtk::model::Entity>();
      smtk::model::Group tmpGrp = grpC.as<smtk::model::Group>();

      BitFlags mask = tmpGrp.membershipMask();
      std::string gName = tmpGrp.name();
      model.removeGroup(tmpGrp);
      pstore->erase(grpC);

      smtk::common::UUID grpUUID = opsession->findOrSetEntityUUID(grpEntity);
      // The group itself should be added too
      grpC = opsession->addCMBEntityToResource(grpUUID, grpEntity, pstore, 1);
      bgroup = grpC.as<smtk::model::Group>();
      bgroup.setMembershipMask(mask);
      bgroup.setName(gName);

      // Add group to model's relationship
      model.addGroup(bgroup);
      modGrps.push_back(bgroup);

      std::cout << "Modified " << grpC.name() << " in " << model.name() << "\n";
    }
  }

  Result result = this->createResult(ok ? smtk::operation::Operation::Outcome::SUCCEEDED
                                        : smtk::operation::Operation::Outcome::FAILED);

  if (ok)
  {
    if (bgroup.isValid())
    {
      smtk::attribute::ComponentItem::Ptr createdEntities = result->findComponent("created");
      if (optype == "Create")
      {
        createdEntities->appendValue(bgroup.component());
      }
      if (!modGrps.empty())
      {
        for (auto c : modGrps)
        {
          createdEntities->appendValue(c.component());
        }
      }
    }
    if (optype == "Remove" && !grpsRemoved.empty())
    {
      smtk::attribute::ComponentItem::Ptr remEntities = result->findComponent("removed");
      for (auto r : grpsRemoved)
      {
        remEntities->appendValue(r.component());
      }
    }
  }

  return result;
}

vtkModelEntity* EntityGroupOperation::fetchCMBCell(
  smtk::session::discrete::Resource::Ptr& resource, const std::string& pname) const
{
  vtkModelItem* item =
    resource->discreteSession()->entityForUUID(const_cast<EntityGroupOperation*>(this)
                                                 ->parameters()
                                                 ->findComponent(pname)
                                                 ->objectValue()
                                                 ->id());

  vtkModelEntity* cell = dynamic_cast<vtkModelEntity*>(item);
  return cell;
}

vtkModelEntity* EntityGroupOperation::fetchCMBCell(smtk::session::discrete::Resource::Ptr& resource,
  const smtk::attribute::ComponentItemPtr& entItem, int idx) const
{
  vtkModelItem* item = resource->discreteSession()->entityForUUID(entItem->objectValue(idx)->id());

  vtkModelEntity* cell = dynamic_cast<vtkModelEntity*>(item);
  return cell;
}

bool EntityGroupOperation::modifyGroup(smtk::session::discrete::Resource::Ptr& resource,
  vtkDiscreteModelWrapper* modelWrapper, vtkModelEntity* grpEntity, bool newGroup,
  smtk::model::EntityRefArray& modGrps)
{
  // for a new group, it is ok to be empty (nothing happens here)
  bool ok = newGroup;

  SessionPtr opsession = resource->discreteSession();

  if (grpEntity && (vtkModelMaterial::SafeDownCast(grpEntity) ||
                     vtkDiscreteModelEntityGroup::SafeDownCast(grpEntity)))
  {
    smtk::model::Model model = this->parameters()->associations()->valueAs<smtk::model::Entity>();

    vtkModelMaterial* grpDS = vtkModelMaterial::SafeDownCast(grpEntity);
    vtkDiscreteModelEntityGroup* grpBC = vtkDiscreteModelEntityGroup::SafeDownCast(grpEntity);
    if (grpDS)
    {
      m_opDomain->ClearGeometricEntitiesToAdd();
      m_opDomain->ClearGeometricEntitiesToRemove();
      m_opDomain->SetId(grpEntity->GetUniquePersistentId());
    }
    else if (grpBC)
    {
      m_opBoundary->ClearEntitiesToAdd();
      m_opBoundary->ClearEntitiesToRemove();
      m_opBoundary->SetId(grpEntity->GetUniquePersistentId());
    }
    auto entItem = this->parameters()->findComponent("cell to add");
    if (entItem)
    {
      for (std::size_t idx = 0; idx < entItem->numberOfValues(); idx++)
      {
        vtkModelEntity* modEntity = this->fetchCMBCell(resource, entItem, static_cast<int>(idx));
        if (modEntity == nullptr)
          continue;
        if (grpDS)
          m_opDomain->AddModelGeometricEntity(modEntity->GetUniquePersistentId());
        else if (grpBC)
          m_opBoundary->AddModelEntity(modEntity->GetUniquePersistentId());
      }
    }

    entItem = this->parameters()->findComponent("cell to remove");
    if (entItem)
    {
      for (std::size_t idx = 0; idx < entItem->numberOfValues(); idx++)
      {
        vtkModelEntity* modEntity = this->fetchCMBCell(resource, entItem, static_cast<int>(idx));
        if (!modEntity)
          continue;
        if (grpDS)
          m_opDomain->RemoveModelGeometricEntity(modEntity->GetUniquePersistentId());
        else if (grpBC)
          m_opBoundary->RemoveModelEntity(modEntity->GetUniquePersistentId());
      }
    }

    if (grpDS)
    {
      m_opDomain->Operate(modelWrapper);
      ok = m_opDomain->GetOperateSucceeded() != 0;

      // if we are dealing with domain set, the entities can only belong
      // to one vtkModelMaterial, and the vtkMaterialOperation will remove
      // the relationship to previous materials from input entities.
      // Therefore, we need to put the previous materials in "modified"
      // item in result.
      if (ok)
      {
        vtkIdList* prevMaterials = m_opDomain->GetPreviousMaterialsOfGeometricEntities();
        for (int i = 0; i < prevMaterials->GetNumberOfIds(); i++)
        {
          vtkModelEntity* matEntity =
            modelWrapper->GetModelEntity(vtkModelMaterialType, prevMaterials->GetId(i));
          smtk::common::UUID prevGrpId = opsession->findOrSetEntityUUID(matEntity);
          smtk::model::EntityRef prevGrpRef(resource, prevGrpId);
          smtk::model::Group prevGrp = prevGrpRef.as<smtk::model::Group>();
          if (!prevGrp.isValid())
            continue;

          BitFlags prevMask = prevGrp.membershipMask();
          std::string prevName = prevGrp.name();
          model.removeGroup(prevGrp);
          resource->erase(prevGrp);

          prevGrpId = opsession->findOrSetEntityUUID(matEntity);
          // The group itself should be added too
          smtk::model::EntityRef grpPrev =
            opsession->addCMBEntityToResource(prevGrpId, matEntity, resource, 1);
          smtk::model::Group tmpgroup = grpPrev.as<smtk::model::Group>();
          tmpgroup.setMembershipMask(prevMask);
          tmpgroup.setName(prevName);

          // Add group to model's relationship
          model.addGroup(tmpgroup);
          modGrps.push_back(tmpgroup);
        }
      }
    }
    else if (grpBC)
    {
      m_opBoundary->Operate(modelWrapper);
      ok = m_opBoundary->GetOperateSucceeded() != 0;
    }
  }

  return ok;
}

const char* EntityGroupOperation::xmlDescription() const
{
  return EntityGroupOperation_xml;
}

} // namespace discrete
} // namespace session
} // namespace smtk
