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
