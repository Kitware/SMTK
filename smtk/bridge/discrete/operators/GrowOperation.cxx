//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "GrowOperation.h"

#include "smtk/bridge/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/io/Logger.h"

#include "smtk/model/Events.h"
#include "smtk/model/Face.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Volume.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModel.h"
#include "vtkModel.h"
#include "vtkModelRegion.h"

#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkPolyData.h"
#include "vtkSelectionNode.h"
#include "vtkSelectionSource.h"

#include "GrowOperation_xml.h"

using namespace smtk::model;
using namespace smtk::attribute;

namespace smtk
{
namespace bridge
{

namespace discrete
{

GrowOperation::GrowOperation()
{
}

bool GrowOperation::ableToOperate()
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

  // There must be a MeshEntity for input selection
  if (!this->parameters()->findMeshSelection("selection"))
  {
    return false;
  }

  return true;
}

bool GrowOperation::writeSelectionResult(
  const std::map<smtk::common::UUID, std::set<int> >& cachedSelection,
  GrowOperation::Result& result)
{
  smtk::attribute::MeshSelectionItem::Ptr outSelectionItem = result->findMeshSelection("selection");
  if (!outSelectionItem)
  {
    std::cerr << "ERROR: no \"selection\" item to write to!" << std::endl;
    return false;
  }

  smtk::attribute::MeshSelectionItem::const_sel_map_it mapIt;
  for (mapIt = cachedSelection.begin(); mapIt != cachedSelection.end(); ++mapIt)
  {
    outSelectionItem->setValues(mapIt->first, mapIt->second);
  }
  return true;
}

void GrowOperation::writeSplitResult(vtkSelectionSplitOperation* splitOp,
  vtkDiscreteModelWrapper* modelWrapper, smtk::bridge::discrete::Resource::Ptr& resource,
  Session* opsession, GrowOperation::Result& result)
{
  // Two comoponents array [sourceFaceId, newFaceId],
  // One sourceFace could have been split into multiple new faces
  vtkIdTypeArray* splitPairArray = splitOp->GetModifiedPairIDs();
  std::map<vtkIdType, std::set<vtkIdType> > splitFaces;
  smtk::common::UUID faceUUID;
  for (vtkIdType i = 0; i < splitPairArray->GetNumberOfTuples(); ++i)
  {
    vtkIdType origId = splitPairArray->GetValue(2 * i);
    splitFaces[origId].insert(splitPairArray->GetValue(2 * i + 1));
  }

  smtk::model::ManagerPtr store = std::static_pointer_cast<smtk::model::Manager>(resource);
  // Adding "created" to the "created" item, as a convenient method
  // to get newly created faces from result.
  // The faces have been split will be in the "modified" item.
  smtk::model::EntityRefArray modEnts;
  smtk::model::EntityRefArray newEnts;

  std::map<vtkIdType, std::set<vtkIdType> >::const_iterator it;
  for (it = splitFaces.begin(); it != splitFaces.end(); ++it)
  {
    vtkModelFace* origFace =
      vtkModelFace::SafeDownCast(modelWrapper->GetModelEntity(vtkModelFaceType, it->first));
    if (!origFace)
      continue;

    faceUUID = opsession->findOrSetEntityUUID(origFace);
    smtk::common::UUID modelid = opsession->findOrSetEntityUUID(modelWrapper->GetModel());
    smtk::model::Model inModel(store, modelid);
    // this will remove and re-add the model so that the model topology and all
    // relationships will be reset properly.
    opsession->retranscribeModel(inModel);

    faceUUID = opsession->findOrSetEntityUUID(origFace);
    modEnts.push_back(smtk::model::EntityRef(store, faceUUID)); // original face

    for (std::set<vtkIdType>::const_iterator fit = it->second.begin(); fit != it->second.end();
         ++fit)
    {
      vtkModelFace* face =
        dynamic_cast<vtkModelFace*>(modelWrapper->GetModelEntity(vtkModelFaceType, *fit));
      faceUUID = opsession->findOrSetEntityUUID(face);
      newEnts.push_back(smtk::model::EntityRef(store, faceUUID)); // new face
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

void GrowOperation::convertToGrowSelection(
  const smtk::attribute::MeshSelectionItemPtr& inSelectionItem, vtkSelection* outSelection,
  Session* opsession)
{
  outSelection->Initialize();
  vtkNew<vtkSelectionNode> selNode;
  vtkInformation* oProperties = selNode->GetProperties();
  oProperties->Set(vtkSelectionNode::CONTENT_TYPE(), vtkSelectionNode::INDICES);
  oProperties->Set(vtkSelectionNode::FIELD_TYPE(), vtkSelectionNode::CELL);
  outSelection->AddNode(selNode.GetPointer());

  vtkNew<vtkIdTypeArray> outSelectionList;
  smtk::attribute::MeshSelectionItem::const_sel_map_it mapIt;
  for (mapIt = inSelectionItem->begin(); mapIt != inSelectionItem->end(); ++mapIt)
  {
    vtkDiscreteModelFace* face =
      vtkDiscreteModelFace::SafeDownCast(opsession->entityForUUID(mapIt->first));
    if (!face)
    {
      std::cout << "Could not get model face with Id: " << mapIt->first << std::endl;
      //vtkErrorMacro("Could not get model face with Id " << faceId);
      continue;
    }
    vtkPolyData* geometry = vtkPolyData::SafeDownCast(face->GetGeometry());
    vtkIdTypeArray* masterCellIds = vtkIdTypeArray::SafeDownCast(geometry->GetCellData()->GetArray(
      vtkDiscreteModelGeometricEntity::GetReverseClassificationArrayName()));
    if (masterCellIds)
    {
      std::set<int>::const_iterator it;
      for (it = mapIt->second.begin(); it != mapIt->second.end(); ++it)
        outSelectionList->InsertNextValue(masterCellIds->GetValue(*it));
    }
  }
  selNode->SetSelectionList(outSelectionList.GetPointer());
}

// This grow_selection is a list of cell ids from master polydata,
// so we need to convert that to the format of
// <FaceUUID, 'set' of cellIds on that face>
bool GrowOperation::convertAndResetOutSelection(
  vtkSelection* inSelection, vtkDiscreteModelWrapper* modelWrapper, Session* opsession)
{
  this->m_outSelection.clear();
  if (inSelection)
  {
    // in the format of list of [composite_index, process_id, index] repeated
    vtkDiscreteModel* model = modelWrapper->GetModel();
    vtkDiscreteModel::ClassificationType& classified = model->GetMeshClassification();
    smtk::common::UUID faceUUID;
    vtkIdType masterId, faceCellId;
    // Gather up cells for each existing model face that are in the selection
    // the CellIds are with respect to the master grid.
    std::map<vtkModelEntity*, std::set<vtkIdType> > entityCellIds;
    for (unsigned int ui = 0; ui < inSelection->GetNumberOfNodes(); ui++)
    {
      vtkSelectionNode* selectionNode = inSelection->GetNode(ui);
      vtkInformation* nodeProperties = selectionNode->GetProperties();
      if (vtkSelectionNode::INDICES != nodeProperties->Get(vtkSelectionNode::CONTENT_TYPE()) ||
        vtkSelectionNode::CELL != nodeProperties->Get(vtkSelectionNode::FIELD_TYPE()))
      {
        std::cout << "Possible problem with selected entities.\n";
        continue;
      }
      vtkIdTypeArray* cellIds = vtkIdTypeArray::SafeDownCast(selectionNode->GetSelectionList());
      if (cellIds)
      {
        for (vtkIdType j = 0; j < cellIds->GetNumberOfTuples(); j++)
        {
          masterId = cellIds->GetValue(j);
          vtkDiscreteModelGeometricEntity* cmbEntity = classified.GetEntity(masterId);
          faceCellId = classified.GetEntityIndex(masterId);

          vtkModelEntity* entity = cmbEntity->GetThisModelEntity();
          faceUUID = opsession->findOrSetEntityUUID(entity);
          this->m_outSelection[faceUUID].insert(faceCellId);
        }
      }
      else
      {
        std::cout << "cellIds is null in vtkSelectionSplitOperation.cxx\n";
      }
    }
    // We should now have all of the selected cell Ids sorted out with the
    // proper model entity they are classified against. So now we can split.
  }

  return true;
}

bool GrowOperation::copyToOutSelection(const smtk::attribute::MeshSelectionItemPtr& inSelectionItem)
{
  smtk::attribute::MeshSelectionItem::const_sel_map_it mapIt;
  for (mapIt = inSelectionItem->begin(); mapIt != inSelectionItem->end(); ++mapIt)
    m_outSelection[mapIt->first] = mapIt->second;
  return true;
}

/// Recursively find all the visible faces
void GrowOperation::findVisibleModelFaces(
  const CellEntity& cellent, std::set<vtkIdType>& visibleFaces, Session* opsession)
{
  CellEntities cellents =
    cellent.isModel() ? cellent.as<smtk::model::Model>().cells() : cellent.boundingCells();
  for (CellEntities::const_iterator it = cellents.begin(); it != cellents.end(); ++it)
  {
    if (it->isFace() && ((it->hasVisibility() && it->visible()) || !it->hasVisibility()))
    {
      vtkModelFace* origFace = vtkModelFace::SafeDownCast(opsession->entityForUUID(it->entity()));
      if (origFace)
        visibleFaces.insert(origFace->GetUniquePersistentId());
    }
    if ((*it).boundingCells().size() > 0)
    {
      this->findVisibleModelFaces(*it, visibleFaces, opsession);
    }
  }
}

GrowOperation::Result GrowOperation::operateInternal()
{
  smtk::model::Model model =
    this->parameters()->findModelEntity("model")->value().as<smtk::model::Model>();

  smtk::bridge::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::bridge::discrete::Resource>(model.component()->resource());

  SessionPtr opsession = resource->discreteSession();

  vtkDiscreteModelWrapper* modelWrapper = opsession->findModelEntity(model.entity());
  bool ok = false;
  smtk::attribute::MeshSelectionItem::Ptr inSelectionItem =
    this->parameters()->findMeshSelection("selection");
  MeshModifyMode opType = inSelectionItem->modifyMode();
  int numSelValues = static_cast<int>(inSelectionItem->numberOfValues());
  double featureAngle = this->parameters()->findDouble("feature angle")->value();

  switch (opType)
  {
    case ACCEPT:
      // convert current outSelection to grow Selection
      this->convertToGrowSelection(inSelectionItem, m_growSelection.GetPointer(), opsession.get());
      // Use current selection to split faces if necessary
      this->m_splitOp->Operate(modelWrapper, this->m_growSelection.GetPointer());
      ok = this->m_splitOp->GetOperateSucceeded() != 0;
      break;
    case RESET:
    case MERGE:
    case SUBTRACT:

      // if the ctrl key is down or multiple cells are selected from rubber band,
      // only do modification of current grow selection using the input selection.
      if (inSelectionItem->isCtrlKeyDown() || numSelValues > 1)
      {
        smtkInfoMacro(log(), "Multiple cells are selected. Modifying current grow selection using "
                             "the input selection.");
        ok = this->copyToOutSelection(inSelectionItem);
      }
      else if (numSelValues == 1) // one cell is clicked.
                                  // do grow
      {
        smtkInfoMacro(
          log(), "One cell is selected. Grow mesh selection constrainted by feature angle "
            << featureAngle << ".");
        std::set<vtkIdType> visModelFaceIds;
        this->findVisibleModelFaces(model, visModelFaceIds, opsession.get());
        this->m_growOp->SetModelWrapper(modelWrapper);
        // NOTE:
        // The fact that operators are state-less, we can NOT cache
        // things inside the operator itself. In this case, the m_growSelection
        // really is empty because a new grow operator is recontructed
        // on the server every time the operator is triggered, so for now, we only
        // do grow, and return the result form grow,
        // then grow+/grow- has to be handled fromm applicaiton with the new grow result,
        // meaing the application has to cache the exiting selection, and based
        // on grow+/grow-, update selection properly
        /*
        // convert current outSelection to grow Selection
        this->convertToGrowSelection(
          inSelectionItem, m_growSelection.GetPointer(), opsession);
        int mode = opType == MeshSelectionItem::RESET ? 0 :
          (opType == MeshSelectionItem::MERGE ? 1 : 2);
*/

        m_growSelection->Initialize();
        int mode = 0;

        this->m_growOp->SetGrowMode(mode);
        this->m_growOp->SetFeatureAngle(featureAngle);
        this->m_growOp->SetInputSelection(this->m_growSelection.GetPointer());
        this->m_growOp->SetGrowFaceIds(visModelFaceIds);

        vtkModelFace* face =
          vtkModelFace::SafeDownCast(opsession->entityForUUID(inSelectionItem->begin()->first));
        if (face)
        {
          this->m_growOp->SetFaceCellId(
            face->GetUniquePersistentId(), *(inSelectionItem->begin()->second.begin()));
          this->m_growOp->Update();
          ok =
            this->convertAndResetOutSelection(m_growOp->GetOutput(), modelWrapper, opsession.get());
        }
      }

      break;
    case NONE:
      this->m_outSelection.clear();
      ok = true; // stop grow
      break;
    default:
      std::cerr << "ERROR: Unrecognized MeshModifyMode: " << std::endl;
      break;
  }

  Result result = this->createResult(ok ? smtk::operation::Operation::Outcome::SUCCEEDED
                                        : smtk::operation::Operation::Outcome::FAILED);

  if (ok)
  {
    switch (opType)
    {
      case ACCEPT:
        this->writeSplitResult(
          m_splitOp.GetPointer(), modelWrapper, resource, opsession.get(), result);
        break;
      case RESET:
      case MERGE:
      case SUBTRACT:
      {
        smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");
        modified->appendValue(model.component());

        this->writeSelectionResult(m_outSelection, result);
        break;
      }
      default:
        break;
    }
  }

  return result;
}

const char* GrowOperation::xmlDescription() const
{
  return GrowOperation_xml;
}

} // namespace discrete
} // namespace bridge
} // namespace smtk
