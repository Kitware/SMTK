/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkGMSSolidReader.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCmbModelSelectionSource.h"

#include "vtkDiscreteModelWrapper.h"
#include "vtkCompositeDataIterator.h"
#include "vtkModelEntity.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSelectionSource.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include <set>

class vtkCmbModelSelectionSource::vtkInternal
{
public:
  typedef std::set<vtkIdType> IDSetType;
  IDSetType SelectedEntityIds;
};

vtkStandardNewMacro(vtkCmbModelSelectionSource);
vtkCxxSetObjectMacro(vtkCmbModelSelectionSource, ModelWrapper, vtkDiscreteModelWrapper);

//-----------------------------------------------------------------------------
vtkCmbModelSelectionSource::vtkCmbModelSelectionSource()
{
  this->Internal = new vtkInternal();
  this->Source = vtkSelectionSource::New();
  this->Selection = vtkSelection::New();
  this->SetNumberOfInputPorts(0);
  this->ModelWrapper = 0;
}

//-----------------------------------------------------------------------------
vtkCmbModelSelectionSource::~vtkCmbModelSelectionSource()
{
  this->SetModelWrapper(0);
  this->Source->Delete();
  this->Selection->Delete();
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void vtkCmbModelSelectionSource::CopyData(vtkSelection *selection)
{
  if(this->Internal->SelectedEntityIds.size()>0)
    {
    this->Internal->SelectedEntityIds.clear();
    }
  this->Selection->ShallowCopy( selection );
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbModelSelectionSource::AddSelectedEntityId(vtkIdType SelectedEntityId)
{
  this->Internal->SelectedEntityIds.insert(SelectedEntityId);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbModelSelectionSource::RemoveAllSelectedEntityIds()
{
  this->RemoveAllSelectedEntityIdsInternal();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbModelSelectionSource::RemoveAllSelectedEntityIdsInternal()
{
  if(this->Internal->SelectedEntityIds.size()>0)
    {
    this->Internal->SelectedEntityIds.clear();
    }
}

//-----------------------------------------------------------------------------
int vtkCmbModelSelectionSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  if(!this->ModelWrapper)
    {
    vtkErrorMacro("Must set ModelWrapper.");
    return 0;
    }
  // get the ouptut
  vtkSelection* output = vtkSelection::GetData(outputVector);
  output->Initialize();

  int piece = 0;
  int npieces = -1;
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  if (outInfo->Has(
    vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()))
    {
    piece = outInfo->Get(
      vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER());
    }
  if (outInfo->Has(
    vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES()))
    {

    npieces = outInfo->Get(
      vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());
    }

  vtkStreamingDemandDrivenPipeline* sddp = vtkStreamingDemandDrivenPipeline::SafeDownCast(
    this->Source->GetExecutive());
  if (sddp)
    {
    sddp->SetUpdateExtent(0, piece, npieces, 0);
    }

  this->Source->SetContentType(vtkSelectionNode::BLOCKS);
  this->Source->RemoveAllBlocks();

  unsigned int curr_idx;
  for(std::set<vtkIdType>::iterator sit=this->Internal->SelectedEntityIds.begin();
    sit!=this->Internal->SelectedEntityIds.end();sit++)
    {
    vtkModelEntity* entity = this->ModelWrapper->GetModelEntity(*sit);
    if(entity && entity->GetVisibility() &&
      this->ModelWrapper->GetChildIndexByEntityId(*sit, curr_idx))
      {
      curr_idx++; // Need to be flat composite index for selection
      this->Source->AddBlock(curr_idx);
      }
    }

  if(this->Internal->SelectedEntityIds.size()>0)
    {
    this->Source->Update();
    this->Selection->ShallowCopy(this->Source->GetOutput());
    }

  // now move the input through to the output
  output->ShallowCopy( this->Selection );
  this->RemoveAllSelectedEntityIdsInternal();
  return 1;
}

//-----------------------------------------------------------------------------
void vtkCmbModelSelectionSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Source: " << this->Source << "\n";
}
