//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkSelectionSplitOperationBase.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkSelectionSplitOperationBase);

vtkSelectionSplitOperationBase::vtkSelectionSplitOperationBase()
{
  this->ModifiedPairIDs = vtkIdTypeArray::New();
  this->ModifiedPairIDs->SetNumberOfComponents(2);
  this->ModifiedPairIDs->SetNumberOfTuples(0);
  this->CompletelySelectedIDs = vtkIdTypeArray::New();
  this->CompletelySelectedIDs->SetNumberOfComponents(1);
  this->CompletelySelectedIDs->SetNumberOfTuples(0);
  this->CurrentExistingFaceId = -1;
}

vtkSelectionSplitOperationBase::~vtkSelectionSplitOperationBase()
{
  if (this->ModifiedPairIDs)
  {
    ModifiedPairIDs->Delete();
    ModifiedPairIDs = nullptr;
  }
  if (this->CompletelySelectedIDs)
  {
    CompletelySelectedIDs->Delete();
    CompletelySelectedIDs = nullptr;
  }
}

void vtkSelectionSplitOperationBase::AddModifiedPair(vtkIdType SourceID, vtkIdType TargetID)
{
  vtkIdType Ids[2] = { SourceID, TargetID };
  this->ModifiedPairIDs->InsertNextTypedTuple(Ids);
}

bool vtkSelectionSplitOperationBase::GetModifiedPair(
  vtkIdType index, vtkIdType& SourceID, vtkIdType& TargetID)
{
  if (index < 0 || index >= this->ModifiedPairIDs->GetNumberOfTuples())
  {
    return false;
  }
  SourceID = this->ModifiedPairIDs->GetValue(index * 2);
  TargetID = this->ModifiedPairIDs->GetValue(index * 2 + 1);
  return true;
}

bool vtkSelectionSplitOperationBase::AbleToOperate(vtkDiscreteModel* Model)
{
  if (!Model)
  {
    vtkErrorMacro("Passed in a null model.");
    return false;
  }

  return true;
}

void vtkSelectionSplitOperationBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ModifiedPairIDs: " << this->ModifiedPairIDs << endl;
  os << indent << "CompletelySelectedIDs: " << this->CompletelySelectedIDs << endl;
}
