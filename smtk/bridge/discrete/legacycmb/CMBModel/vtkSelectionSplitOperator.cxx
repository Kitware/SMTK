//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkSelectionSplitOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSelectionSource.h"

//#define CMB_WRITE_MODEL
#ifdef CMB_WRITE_MODEL
#include "vtkCMBModelWriterBase.h"
#include "vtkNew.h"
#endif

#include <set>

vtkStandardNewMacro(vtkSelectionSplitOperator);

vtkSelectionSplitOperator::vtkSelectionSplitOperator()
{
  this->OperateSucceeded = 0;
}

vtkSelectionSplitOperator::~vtkSelectionSplitOperator()
{
}

void vtkSelectionSplitOperator::Operate(
  vtkDiscreteModelWrapper* modelWrapper, vtkSelectionAlgorithm* selectionSource)
{
  this->OperateSucceeded = 0;
  if (this->AbleToOperate(modelWrapper) == 0 || selectionSource == NULL)
  {
    return;
  }
  selectionSource->Update();
  vtkSelection* selection = selectionSource->GetOutput();
  if (selection == NULL)
  {
    return;
  }

  vtkDiscreteModel* model = modelWrapper->GetModel();
  vtkDiscreteModel::ClassificationType& classified = model->GetMeshClassification();

  // Gather up cells for each existing model face that are in the selection
  // the CellIds are with respect to the master grid.
  std::map<vtkModelEntity*, std::set<vtkIdType> > entityCellIds;
  for (unsigned int ui = 0; ui < selection->GetNumberOfNodes(); ui++)
  {
    vtkSelectionNode* selectionNode = selection->GetNode(ui);
    vtkInformation* nodeProperties = selectionNode->GetProperties();
    if (vtkSelectionNode::INDICES != nodeProperties->Get(vtkSelectionNode::CONTENT_TYPE()) ||
      vtkSelectionNode::CELL != nodeProperties->Get(vtkSelectionNode::FIELD_TYPE()))
    {
      vtkWarningMacro("Possible problem with selected entities.");
      continue;
    }
    vtkIdTypeArray* cellIds = vtkIdTypeArray::SafeDownCast(selectionNode->GetSelectionList());
    if (cellIds)
    {
      for (vtkIdType j = 0; j < cellIds->GetNumberOfTuples(); j++)
      {
        vtkDiscreteModelGeometricEntity* cmbEntity = classified.GetEntity(cellIds->GetValue(j));
        vtkModelEntity* entity = cmbEntity->GetThisModelEntity();
        entityCellIds[entity].insert(cellIds->GetValue(j));
      }
    }
    else
    {
      cout << "cellIds is null in vtkSelectionSplitOperator.cxx\n";
    }
  }
  // We should now have all of the selected cell Ids sorted out with the
  // proper model entity they are classified against. So now we can split.
  std::set<vtkIdType> newEnts;
  for (std::map<vtkModelEntity*, std::set<vtkIdType> >::iterator it = entityCellIds.begin();
       it != entityCellIds.end(); it++)
  {
    vtkDiscreteModelFace* face = vtkDiscreteModelFace::SafeDownCast(it->first);
    if (!face)
    {
      vtkWarningMacro("Can only deal with model faces currently.");
      continue;
    }
    vtkIdType existingId = face->GetUniquePersistentId();
    vtkIdType numCells = static_cast<vtkIdType>(it->second.size());
    if (face->GetNumberOfCells() > numCells)
    { // this model geometric entity is getting split
      vtkSmartPointer<vtkIdList> cellIdList = vtkSmartPointer<vtkIdList>::New();
      cellIdList->SetNumberOfIds(numCells);
      vtkIdType counter = 0;
      for (std::set<vtkIdType>::iterator sit = it->second.begin(); sit != it->second.end();
           sit++, counter++)
      {
        cellIdList->SetId(counter, *sit);
      }
      this->FaceSplitInfo.insert(std::make_pair(existingId, FaceEdgeSplitInfo()));
      vtkDiscreteModelFace* newFace =
        face->BuildFromExistingModelFace(cellIdList, this->FaceSplitInfo[existingId], true);
      this->AddModifiedPair(existingId, newFace->GetUniquePersistentId());
      newEnts.insert(newFace->GetUniquePersistentId());
      // Also add edges for new faces if they are available
      if (face->GetNumberOfModelEdges())
      {
        face->GetModelEdgeIds(newEnts); // There could be new edges for original face
        newFace->GetModelEdgeIds(newEnts);
        newFace->GetModelVertexIds(newEnts);
      }
    }
    else if (face->GetNumberOfCells() == numCells)
    {
      this->GetCompletelySelectedIDs()->InsertNextValue(face->GetUniquePersistentId());
    }
    else
    {
      vtkErrorMacro("Too many cells to be split from existing entity.");
    }
  }
  modelWrapper->AddGeometricEntities(newEnts);

#ifdef CMB_WRITE_MODEL
  vtkNew<vtkCMBModelWriterBase> writer;
  writer->SetFileName("C:/Temp/modelWithEdges.vtp");
  writer->Operate(modelWrapper);
#endif

  this->OperateSucceeded = 1;
}

bool vtkSelectionSplitOperator::AbleToOperate(vtkDiscreteModelWrapper* modelWrapper)
{
  if (!modelWrapper)
  {
    vtkErrorMacro("Passed in a null model wrapper.");
    return 0;
  }

  return this->Superclass::AbleToOperate(modelWrapper->GetModel());
}

void vtkSelectionSplitOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
