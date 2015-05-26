//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "EntityGroupOperator.h"

#include "smtk/bridge/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdList.h"
#include "vtkModel.h"
#include "vtkModelMaterial.h"

#include "EntityGroupOperator_xml.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {

  namespace discrete {

EntityGroupOperator::EntityGroupOperator()
{
}

bool EntityGroupOperator::ableToOperate()
{
  smtk::model::Model model;
  bool able2Op =
    this->ensureSpecification() &&
    // The SMTK model must be valid
    (model = this->specification()->findModelEntity("model")
      ->value().as<smtk::model::Model>()).isValid() &&
    // The CMB model must exist
    this->discreteSession()->findModelEntity(model.entity());

  if(!able2Op)
    {
    return able2Op;
    }


// for Create operation, we just need model, and default entity
// type will be Face;
// for Destroy and Modify operation, we need cell group is set
  smtk::attribute::StringItem::Ptr optypeItem =
    this->specification()->findString("Operation");

  std::string optype = optypeItem->value();
  // if(optype == "Create") //only need model;
  if(optype == "Remove")
    able2Op = this->fetchCMBCell("remove cell group") != NULL;
  else if(optype == "Modify")
    able2Op = this->fetchCMBCell("modify cell group") != NULL && (
    this->fetchCMBCell("cell to add") ||
    this->fetchCMBCell("cell to remove") );

  return able2Op;
}

int EntityGroupOperator::createBoundaryGroup(vtkDiscreteModelWrapper* modelWrapper)
{
  int entType = modelWrapper->GetModel()->GetModelDimension() == 3 ?
    vtkModelFaceType :
    (modelWrapper->GetModel()->GetModelDimension() == 2 ?
     vtkModelEdgeType : -1);
  if (entType == -1)
    return -1;

  this->m_opBoundary->SetBuildEnityType(entType);
  this->m_opBoundary->Build(modelWrapper);
  int grpId = this->m_opBoundary->GetBuiltModelEntityGroupId();
  return grpId;
}

int EntityGroupOperator::createDomainSet(vtkDiscreteModelWrapper* modelWrapper)
{
  this->m_opDomain->Build(modelWrapper);
  int dsId = this->m_opDomain->GetBuiltMaterialId();
  return dsId;
}

OperatorResult EntityGroupOperator::operateInternal()
{
  smtk::model::ManagerPtr pstore = this->manager();
  Session* opsession = this->discreteSession();
  // ableToOperate should have verified that model is valid
  smtk::model::Model model = this->specification()->
    findModelEntity("model")->value().as<smtk::model::Model>();
  vtkDiscreteModelWrapper* modelWrapper =
    opsession->findModelEntity(model.entity());
  bool ok = false;
  smtk::model::Group bgroup;
  smtk::model::EntityRefArray modGrps;
  smtk::model::EntityRefs grpsRemoved;
  // Translate SMTK inputs into CMB inputs
  smtk::attribute::StringItem::Ptr optypeItem =
    this->specification()->findString("Operation");
  std::string optype = optypeItem->value();
  if(optype == "Create") //only need model
    {
    int gType = this->specification()->findInt("group type")->value();
    std::string gName = this->specification()->findString("group name")->value();

    int grpId = -1;
    if(gType == 0) // boundary group
      grpId = this->createBoundaryGroup(modelWrapper);
    else if(gType == 1) // domain set
      grpId = this->createDomainSet(modelWrapper);
    ok = grpId>=0;

    if(ok)
      {
      BitFlags mask =
       // Boundary group, 3d => Face group; 2d => edge group.
        (gType == 0) ?
          (modelWrapper->GetModel()->GetModelDimension() == 3 ?
            smtk::model::FACE : smtk::model::EDGE) :
       // Domain set, 3d => volume group; 2d => face group.
        (modelWrapper->GetModel()->GetModelDimension() == 3 ?
            smtk::model::VOLUME : smtk::model::FACE);
      int groupType = gType == 0 ?
        vtkDiscreteModelEntityGroupType : vtkModelMaterialType;
      vtkModelEntity* grp =
        modelWrapper->GetModelEntity(groupType, grpId);
      smtk::common::UUID grpUUID = opsession->findOrSetEntityUUID(grp);

      // The group itself should be added too
      smtk::model::EntityRef grpRef = opsession->addCMBEntityToManager(
                                      grpUUID, grp, pstore, 0);
      bgroup = grpRef.as<smtk::model::Group>();
      bgroup.setMembershipMask(mask);
      bgroup.setName(gName);
      // Add group to model's relationship
      model.addGroup(bgroup);
      modGrps.push_back(bgroup);
      std::cout << "new group: " << bgroup.name() << " id: " << grpUUID.toString() << "\n";
      }
    }
  else if(optype == "Remove")
    {
    smtk::attribute::ModelEntityItemPtr remgrpItem =
      this->specification()->findModelEntity("remove cell group");
    for(std::size_t idx=0; idx<remgrpItem->numberOfValues(); idx++)
      {
      vtkModelEntity* modEntity = this->fetchCMBCell(remgrpItem, idx);
      if(!modEntity)
        continue;
      if(modEntity->GetType() == vtkModelMaterialType)
        {
        this->m_opDomain->SetId(modEntity->GetUniquePersistentId());
        this->m_opDomain->Destroy(modelWrapper);
        ok = this->m_opDomain->GetDestroySucceeded();
        }
      else if(modEntity->GetType() == vtkDiscreteModelEntityGroupType)
        {
        this->m_opBoundary->SetId(modEntity->GetUniquePersistentId());
        this->m_opBoundary->Destroy(modelWrapper);
        ok = this->m_opBoundary->GetDestroySucceeded();
        }
      if(ok)
        {
        // get rid of the group from manager
        smtk::model::EntityRef grpRem = remgrpItem->value(idx);
        model.removeGroup(grpRem.as<smtk::model::Group>());
        pstore->erase(grpRem);
        std::cout << "Removed " << grpRem.name() << " to " << model.name() << "\n";
        grpsRemoved.insert(grpRem);
        }
      }
    }
  else if(optype == "Modify")
    {
    vtkModelEntity* grpEntity = this->fetchCMBCell("modify cell group");
    if(grpEntity && (vtkModelMaterial::SafeDownCast(grpEntity) ||
       vtkDiscreteModelEntityGroup::SafeDownCast(grpEntity)))
      {
      vtkModelMaterial* grpDS = vtkModelMaterial::SafeDownCast(grpEntity);
      vtkDiscreteModelEntityGroup* grpBC = vtkDiscreteModelEntityGroup::SafeDownCast(grpEntity);
      if(grpDS)
        {
        this->m_opDomain->ClearGeometricEntitiesToAdd();
        this->m_opDomain->ClearGeometricEntitiesToRemove();
        this->m_opDomain->SetId(grpEntity->GetUniquePersistentId());
        }
      else if(grpBC)
        {
        this->m_opBoundary->ClearEntitiesToAdd();
        this->m_opBoundary->ClearEntitiesToRemove();
        this->m_opBoundary->SetId(grpEntity->GetUniquePersistentId());
        }
      smtk::attribute::ModelEntityItemPtr entItem =
        this->specification()->findModelEntity("cell to add");
      for(std::size_t idx=0; idx<entItem->numberOfValues(); idx++)
        {
        vtkModelEntity* modEntity = this->fetchCMBCell(entItem, idx);
        if(!modEntity)
          continue;
        if(grpDS)
          this->m_opDomain->AddModelGeometricEntity(modEntity->GetUniquePersistentId());
        else if(grpBC)
           this->m_opBoundary->AddModelEntity(modEntity->GetUniquePersistentId());
        }

      entItem = this->specification()->findModelEntity("cell to remove");
      for(std::size_t idx=0; idx<entItem->numberOfValues(); idx++)
        {
        vtkModelEntity* modEntity = this->fetchCMBCell(entItem, idx);
        if(!modEntity)
          continue;
        if(grpDS)
          this->m_opDomain->RemoveModelGeometricEntity(modEntity->GetUniquePersistentId());
        else if(grpBC)
           this->m_opBoundary->RemoveModelEntity(modEntity->GetUniquePersistentId());
        }

      if(grpDS)
        {
        this->m_opDomain->Operate(modelWrapper);
        ok = this->m_opDomain->GetOperateSucceeded();
        }
      else if(grpBC)
        {
        this->m_opBoundary->Operate(modelWrapper);
        ok = this->m_opBoundary->GetOperateSucceeded();
        }

      if(ok)
        {
        // get rid of the group from manager
        smtk::model::EntityRef grpC =
          this->specification()->findModelEntity("modify cell group")->value();
        smtk::model::Group tmpGrp = grpC.as<smtk::model::Group>();

        BitFlags mask = tmpGrp.membershipMask();
        std::string gName = tmpGrp.name();
        model.removeGroup(tmpGrp);
        pstore->erase(grpC);

        smtk::common::UUID grpUUID = opsession->findOrSetEntityUUID(grpEntity);
        // The group itself should be added too
        grpC = opsession->addCMBEntityToManager(
                                        grpUUID, grpEntity, pstore, 1);
        bgroup = grpC.as<smtk::model::Group>();
        bgroup.setMembershipMask(mask);
        bgroup.setName(gName);

        // Add group to model's relationship
        model.addGroup(bgroup);
        modGrps.push_back(bgroup);

        // if we are dealing with domain set, the entities can only belong
        // to one vtkModelMaterial, and the vtkMaterialOperator will remove
        // the relationship to previous materials from input entities.
        // Therefore, we need to put the previous materials in "modified"
        // item in result.
        if(grpDS)
          {
          vtkIdList* prevMaterials = this->m_opDomain->
              GetPreviousMaterialsOfGeometricEntities();
          for(int i=0; i<prevMaterials->GetNumberOfIds();i++)
            {
            vtkModelEntity* matEntity = modelWrapper->GetModelEntity(
              vtkModelMaterialType, prevMaterials->GetId(i));
            smtk::common::UUID prevGrpId = opsession->findOrSetEntityUUID(matEntity);
            smtk::model::EntityRef prevGrpRef(pstore, prevGrpId);
            smtk::model::Group prevGrp = prevGrpRef.as<smtk::model::Group>();
            if(!prevGrp.isValid())
              continue;

            BitFlags prevMask = prevGrp.membershipMask();
            std::string prevName = prevGrp.name();
            model.removeGroup(prevGrp);
            pstore->erase(prevGrp);

            prevGrpId = opsession->findOrSetEntityUUID(matEntity);
            // The group itself should be added too
            smtk::model::EntityRef grpPrev = opsession->addCMBEntityToManager(
                                            prevGrpId, matEntity, pstore, 1);
            smtk::model::Group tmpgroup = grpPrev.as<smtk::model::Group>();
            tmpgroup.setMembershipMask(prevMask);
            tmpgroup.setName(prevName);

            // Add group to model's relationship
            model.addGroup(tmpgroup);
            modGrps.push_back(tmpgroup);

            }
          }

        std::cout << "Modified " << grpC.name() << " in " << model.name() << "\n";
        }
      }

    }

  OperatorResult result =
    this->createResult(
      ok ?  OPERATION_SUCCEEDED : OPERATION_FAILED);

  if (ok)
    {
    if(bgroup.isValid())
      {
      // Return the created or modified group.
      if(optype == "Create")
        this->addEntityToResult(result, bgroup, CREATED);
      else if(optype == "Modify" && modGrps.size() > 0)
        this->addEntitiesToResult(result, modGrps, MODIFIED);
      }
    if(optype == "Remove" && grpsRemoved.size() > 0)
      {
      // Return the created or modified group.
      smtk::attribute::ModelEntityItem::Ptr remEntities =
        result->findModelEntity("expunged");
      remEntities->setNumberOfValues(grpsRemoved.size());
      remEntities->setIsEnabled(true);
      smtk::model::EntityRefs::const_iterator it;
      int rid = 0;
      for (it=grpsRemoved.begin(); it != grpsRemoved.end(); it++)
        remEntities->setValue(rid++, *it);
      }

    }

  return result;
}

Session* EntityGroupOperator::discreteSession() const
{
  return dynamic_cast<Session*>(this->session());
}

vtkModelEntity* EntityGroupOperator::fetchCMBCell(const std::string& pname) const
{
  vtkModelItem* item =
    this->discreteSession()->entityForUUID(
      this->specification()->findModelEntity(pname)->value().entity());

  vtkModelEntity* cell = dynamic_cast<vtkModelEntity*>(item);
  return cell;
}

vtkModelEntity* EntityGroupOperator::fetchCMBCell(
  const smtk::attribute::ModelEntityItemPtr& entItem, int idx ) const
{
  vtkModelItem* item =
    this->discreteSession()->entityForUUID(entItem->value(idx).entity());

  vtkModelEntity* cell = dynamic_cast<vtkModelEntity*>(item);
  return cell;
}

    } // namespace discrete
  } // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(
  SMTKDISCRETESESSION_EXPORT,
  smtk::bridge::discrete::EntityGroupOperator,
  discrete_entity_group,
  "entity group",
  EntityGroupOperator_xml,
  smtk::bridge::discrete::Session);
