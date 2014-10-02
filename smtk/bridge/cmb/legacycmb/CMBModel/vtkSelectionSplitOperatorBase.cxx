//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkSelectionSplitOperatorBase.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkSelectionSplitOperatorBase);

vtkSelectionSplitOperatorBase::vtkSelectionSplitOperatorBase()
{
  this->ModifiedPairIDs = vtkIdTypeArray::New();
  this->ModifiedPairIDs->SetNumberOfComponents(2);
  this->ModifiedPairIDs->SetNumberOfTuples(0);
  this->CompletelySelectedIDs = vtkIdTypeArray::New();
  this->CompletelySelectedIDs->SetNumberOfComponents(1);
  this->CompletelySelectedIDs->SetNumberOfTuples(0);
  this->CurrentExistingFaceId = -1;
}

vtkSelectionSplitOperatorBase::~vtkSelectionSplitOperatorBase()
{
  if(this->ModifiedPairIDs)
    {
    ModifiedPairIDs->Delete();
    ModifiedPairIDs = 0;
    }
  if(this->CompletelySelectedIDs)
    {
    CompletelySelectedIDs->Delete();
    CompletelySelectedIDs = 0;
    }
}

void vtkSelectionSplitOperatorBase::AddModifiedPair(
  vtkIdType SourceID, vtkIdType TargetID)
{
  vtkIdType Ids[2] = {SourceID, TargetID};
  this->ModifiedPairIDs->InsertNextTupleValue(Ids);
}


bool vtkSelectionSplitOperatorBase::GetModifiedPair(
  vtkIdType index, vtkIdType & SourceID, vtkIdType & TargetID)
{
  if(index < 0 || index >= this->ModifiedPairIDs->GetNumberOfTuples())
    {
    return 0;
    }
  SourceID = this->ModifiedPairIDs->GetValue(index*2);
  TargetID = this->ModifiedPairIDs->GetValue(index*2+1);
  return 1;
}

bool vtkSelectionSplitOperatorBase::AbleToOperate(vtkDiscreteModel* Model)
{
  if(!Model)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }

  return 1;
}

void vtkSelectionSplitOperatorBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "ModifiedPairIDs: " << this->ModifiedPairIDs << endl;
  os << indent << "CompletelySelectedIDs: " << this->CompletelySelectedIDs << endl;
}
