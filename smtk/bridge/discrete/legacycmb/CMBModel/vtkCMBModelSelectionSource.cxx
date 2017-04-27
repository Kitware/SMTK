//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelSelectionSource.h"

#include "vtkCompositeDataIterator.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModelEntity.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSelectionSource.h"
#include <set>

class vtkCMBModelSelectionSource::vtkInternal
{
public:
  typedef std::set<vtkIdType> IDSetType;
  IDSetType SelectedEntityIds;
};

vtkStandardNewMacro(vtkCMBModelSelectionSource);
vtkCxxSetObjectMacro(vtkCMBModelSelectionSource, ModelWrapper, vtkDiscreteModelWrapper);

vtkCMBModelSelectionSource::vtkCMBModelSelectionSource()
{
  this->Internal = new vtkInternal();
  this->Source = vtkSelectionSource::New();
  this->Selection = vtkSelection::New();
  this->SetNumberOfInputPorts(0);
  this->ModelWrapper = 0;
}

vtkCMBModelSelectionSource::~vtkCMBModelSelectionSource()
{
  this->SetModelWrapper(0);
  this->Source->Delete();
  this->Selection->Delete();
  delete this->Internal;
}

void vtkCMBModelSelectionSource::CopyData(vtkSelection* selection)
{
  if (this->Internal->SelectedEntityIds.size() > 0)
  {
    this->Internal->SelectedEntityIds.clear();
  }
  this->Selection->ShallowCopy(selection);
  this->Modified();
}

void vtkCMBModelSelectionSource::AddSelectedEntityId(vtkIdType SelectedEntityId)
{
  this->Internal->SelectedEntityIds.insert(SelectedEntityId);
  this->Modified();
}

void vtkCMBModelSelectionSource::RemoveAllSelectedEntityIds()
{
  this->RemoveAllSelectedEntityIdsInternal();
  this->Modified();
}

void vtkCMBModelSelectionSource::RemoveAllSelectedEntityIdsInternal()
{
  if (this->Internal->SelectedEntityIds.size() > 0)
  {
    this->Internal->SelectedEntityIds.clear();
  }
}

int vtkCMBModelSelectionSource::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  if (!this->ModelWrapper)
  {
    vtkErrorMacro("Must set ModelWrapper.");
    return 0;
  }
  // get the ouptut
  vtkSelection* output = vtkSelection::GetData(outputVector);
  output->Initialize();

  int piece = 0;
  int npieces = -1;
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()))
  {
    piece = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER());
  }
  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES()))
  {

    npieces = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());
  }

  vtkStreamingDemandDrivenPipeline* sddp =
    vtkStreamingDemandDrivenPipeline::SafeDownCast(this->Source->GetExecutive());
  if (sddp)
  {
    sddp->SetUpdateExtent(0, piece, npieces, 0);
  }

  this->Source->SetContentType(vtkSelectionNode::BLOCKS);
  this->Source->RemoveAllBlocks();

  unsigned int curr_idx;
  for (std::set<vtkIdType>::iterator sit = this->Internal->SelectedEntityIds.begin();
       sit != this->Internal->SelectedEntityIds.end(); sit++)
  {
    vtkModelEntity* entity = this->ModelWrapper->GetModelEntity(*sit);
    if (entity && entity->GetVisibility() &&
      this->ModelWrapper->GetChildIndexByEntityId(*sit, curr_idx))
    {
      curr_idx++; // Need to be flat composite index for selection
      this->Source->AddBlock(curr_idx);
    }
  }

  if (this->Internal->SelectedEntityIds.size() > 0)
  {
    this->Source->Update();
    this->Selection->ShallowCopy(this->Source->GetOutput());
  }

  // now move the input through to the output
  output->ShallowCopy(this->Selection);
  this->RemoveAllSelectedEntityIdsInternal();
  return 1;
}

void vtkCMBModelSelectionSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Source: " << this->Source << "\n";
}
