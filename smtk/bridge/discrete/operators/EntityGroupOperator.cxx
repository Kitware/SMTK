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
#include "vtkModel.h"

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
    able2Op = this->fetchCMBCellId("remove cell group") >= 0;
  else if(optype == "Modify")
    able2Op = this->fetchCMBCellId("modify cell group") >= 0 && (
    this->fetchCMBCellId("cell to add") >= 0 ||
    this->fetchCMBCellId("cell to remove") >= 0 );

  return able2Op;
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
  smtk::model::EntityRefs grpsRemoved;
  // Translate SMTK inputs into CMB inputs
  smtk::attribute::StringItem::Ptr optypeItem =
    this->specification()->findString("Operation");
  std::string optype = optypeItem->value();
  if(optype == "Create") //only need model
    {
    int gType = this->specification()->findInt("entity type")->value();
    std::string gName = this->specification()->findString("group name")->value();
    int entType = gType==0 ? vtkModelFaceType :
      (gType==1 ? vtkModelEdgeType : vtkModelVertexType);
    this->m_op->SetBuildEnityType(entType);
    this->m_op->Build(modelWrapper);
    int grpId = this->m_op->GetBuiltModelEntityGroupId();
    ok = grpId>=0;
    if(ok)
      {
      BitFlags mask = entType==vtkModelFaceType ? smtk::model::FACE :
        (entType==vtkModelEdgeType ? smtk::model::EDGE : smtk::model::VERTEX);
      vtkDiscreteModelEntityGroup* grp = dynamic_cast<vtkDiscreteModelEntityGroup*>(
        modelWrapper->GetModelEntity(vtkDiscreteModelEntityGroupType, grpId));
      smtk::common::UUID grpUUID = opsession->findOrSetEntityUUID(grp);
      //bgroup = opsession->addGroupToManager(grpUUID, grp, pstore, 0);
      // The group itself should be added too
      smtk::model::EntityRef grpRef = opsession->addCMBEntityToManager(
                                      grpUUID, grp, pstore, 0);
      bgroup = grpRef.as<smtk::model::Group>();
      bgroup.setMembershipMask(mask);
      bgroup.setName(gName);
      // Add group to model's relationship
      model.addGroup(bgroup);
      std::cout << "new group: " << bgroup.name() << " id: " << grpUUID.toString() << "\n";
      }
    }
  else if(optype == "Remove")
    {
    smtk::attribute::ModelEntityItemPtr remgrpItem =
      this->specification()->findModelEntity("remove cell group");
    for(std::size_t idx=0; idx<remgrpItem->numberOfValues(); idx++)
      {
      int grpid = this->fetchCMBCellId(remgrpItem, idx);
      if(grpid >= 0)
        {
        this->m_op->SetId(grpid);
        this->m_op->Destroy(modelWrapper);
        ok = this->m_op->GetDestroySucceeded();
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

    }
  else if(optype == "Modify")
    {
    int grpId = this->fetchCMBCellId("modify cell group");
    this->m_op->SetId(grpId);
    smtk::attribute::ModelEntityItemPtr entItem =
      this->specification()->findModelEntity("cell to add");
    for(std::size_t idx=0; idx<entItem->numberOfValues(); idx++)
      {
      int cmbid = this->fetchCMBCellId(entItem, idx);
      if(cmbid >= 0)
        this->m_op->AddModelEntity(cmbid);
      }

    entItem = this->specification()->findModelEntity("cell to remove");
    for(std::size_t idx=0; idx<entItem->numberOfValues(); idx++)
      {
      int cmbid = this->fetchCMBCellId(entItem, idx);
      if(cmbid >= 0)
        this->m_op->RemoveModelEntity(cmbid);
      }
    this->m_op->Operate(modelWrapper);
    ok = this->m_op->GetOperateSucceeded();
    if(ok)
      {
      vtkDiscreteModelEntityGroup* grp = dynamic_cast<vtkDiscreteModelEntityGroup*>(
        modelWrapper->GetModelEntity(vtkDiscreteModelEntityGroupType, grpId));
      // get rid of the group from manager
      smtk::model::EntityRef grpC =
        this->specification()->findModelEntity("modify cell group")->value();
      smtk::model::Group tmpGrp = grpC.as<smtk::model::Group>();
      BitFlags mask = tmpGrp.membershipMask();
      std::string gName = tmpGrp.name();
      model.removeGroup(tmpGrp);
      pstore->erase(grpC);

//      bgroup = opsession->addGroupToManager(grpC.entity(), grp, pstore, true);
      smtk::common::UUID grpUUID = opsession->findOrSetEntityUUID(grp);
      // The group itself should be added too
      grpC = opsession->addCMBEntityToManager(
                                      grpUUID, grp, pstore, 1);
      bgroup = grpC.as<smtk::model::Group>();
      bgroup.setMembershipMask(mask);
      bgroup.setName(gName);

      // Add group to model's relationship
      model.addGroup(bgroup);

      std::cout << "Modified " << grpC.name() << " in " << model.name() << "\n";
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
      smtk::attribute::ModelEntityItem::Ptr entities =
        result->findModelEntity("entities");
      entities->setNumberOfValues(1);
      entities->setValue(0, bgroup);
     if(optype == "Create")
       {
       // Adding the new group to the "new entities" item, as a convenient method
       // to get newly created group from result. This group is also listed in the
       // "entities" item.
       smtk::attribute::ModelEntityItem::Ptr newEntities =
          result->findModelEntity("new entities");
       newEntities->setNumberOfValues(1);
       newEntities->setValue(0, bgroup);
       }
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

int EntityGroupOperator::fetchCMBCellId(const std::string& pname) const
{
  vtkModelItem* item =
    this->discreteSession()->entityForUUID(
      this->specification()->findModelEntity(pname)->value().entity());

  vtkModelEntity* cell = dynamic_cast<vtkModelEntity*>(item);
  if (cell)
    return cell->GetUniquePersistentId();

  return -1;
}

int EntityGroupOperator::fetchCMBCellId(
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
  smtk::bridge::discrete::EntityGroupOperator,
  discrete_entity_group,
  "entity group",
  EntityGroupOperator_xml,
  smtk::bridge::discrete::Session);
