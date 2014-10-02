/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "vtkSelectionSplitOperator.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkSelectionSource.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"

//#define CMB_WRITE_MODEL
#ifdef CMB_WRITE_MODEL
  #include "vtkNew.h"
  #include "vtkCMBModelWriterBase.h"
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

void vtkSelectionSplitOperator::Operate(vtkDiscreteModelWrapper* modelWrapper,
                                        vtkSelectionAlgorithm* selectionSource)
{
  this->OperateSucceeded = 0;
  if(this->AbleToOperate(modelWrapper) == 0 || selectionSource == NULL)
    {
    return;
    }
  selectionSource->Update();
  vtkSelection* selection = selectionSource->GetOutput();
  if(selection==NULL)
    {
    return;
    }

  vtkDiscreteModel* model = modelWrapper->GetModel();
  vtkDiscreteModel::ClassificationType& classified =
                            model->GetMeshClassification();

  // Gather up cells for each existing model face that are in the selection
  // the CellIds are with respect to the master grid.
  std::map<vtkModelEntity*, std::set<vtkIdType> > entityCellIds;
  for(unsigned int ui=0;ui<selection->GetNumberOfNodes();ui++)
    {
    vtkSelectionNode* selectionNode = selection->GetNode(ui);
    vtkInformation* nodeProperties = selectionNode->GetProperties();
    if(vtkSelectionNode::INDICES !=
       nodeProperties->Get(vtkSelectionNode::CONTENT_TYPE()) ||
       vtkSelectionNode::CELL !=
       nodeProperties->Get(vtkSelectionNode::FIELD_TYPE()))
      {
      vtkWarningMacro("Possible problem with selected entities.");
      continue;
      }
    vtkIdTypeArray* cellIds = vtkIdTypeArray::SafeDownCast(
      selectionNode->GetSelectionList());
    if(cellIds)
      {
      for(vtkIdType j=0;j<cellIds->GetNumberOfTuples();j++)
        {
        vtkDiscreteModelGeometricEntity* cmbEntity =
            classified.GetEntity(cellIds->GetValue(j));
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
  for(std::map<vtkModelEntity*, std::set<vtkIdType> >::iterator it =
        entityCellIds.begin();it!=entityCellIds.end();it++)
    {
    vtkDiscreteModelFace* face = vtkDiscreteModelFace::SafeDownCast(it->first);
    if(!face)
      {
      vtkWarningMacro("Can only deal with model faces currently.");
      continue;
      }
    vtkIdType existingId = face->GetUniquePersistentId();
    vtkIdType numCells = static_cast<vtkIdType>(it->second.size());
    if(face->GetNumberOfCells() > numCells)
      { // this model geometric entity is getting split
      vtkSmartPointer<vtkIdList> cellIdList = vtkSmartPointer<vtkIdList>::New();
      cellIdList->SetNumberOfIds(numCells);
      vtkIdType counter = 0;
      for(std::set<vtkIdType>::iterator sit=it->second.begin();sit!=it->second.end();
        sit++,counter++)
        {
        cellIdList->SetId(counter, *sit);
        }
      this->FaceSplitInfo.insert(std::make_pair(existingId,FaceEdgeSplitInfo()));
      vtkDiscreteModelFace* newFace = face->BuildFromExistingModelFace(
        cellIdList, this->FaceSplitInfo[existingId], true);
      this->AddModifiedPair(existingId,
                            newFace->GetUniquePersistentId());
      newEnts.insert(newFace->GetUniquePersistentId());
      // Also add edges for new faces if they are available
      if(face->GetNumberOfModelEdges())
        {
        face->GetModelEdgeIds(newEnts); // There could be new edges for original face
        newFace->GetModelEdgeIds(newEnts);
        newFace->GetModelVertexIds(newEnts);
        }
      }
    else if(face->GetNumberOfCells() == numCells)
      {
      this->GetCompletelySelectedIDs()->
        InsertNextValue(face->GetUniquePersistentId());
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
  if(!modelWrapper)
    {
    vtkErrorMacro("Passed in a null model wrapper.");
    return 0;
    }

  return this->Superclass::AbleToOperate(modelWrapper->GetModel());
}

void vtkSelectionSplitOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
