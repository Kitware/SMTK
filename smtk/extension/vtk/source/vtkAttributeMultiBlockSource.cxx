//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/source/vtkAttributeMultiBlockSource.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include "smtk/attribute/Resource.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"

using namespace smtk::attribute;

vtkStandardNewMacro(vtkAttributeMultiBlockSource);
smtkImplementTracksAllInstances(vtkAttributeMultiBlockSource);

vtkAttributeMultiBlockSource::vtkAttributeMultiBlockSource()
{
  this->SetNumberOfInputPorts(0);
  this->linkInstance();
}

vtkAttributeMultiBlockSource::~vtkAttributeMultiBlockSource()
{
  this->unlinkInstance();
}

void vtkAttributeMultiBlockSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Resource: " << this->GetResource().get() << "\n";
}

/// Set the SMTK model to be displayed.
void vtkAttributeMultiBlockSource::SetAttributeResource(const smtk::attribute::ResourcePtr& attr)
{
  this->SetResource(attr);
}

/// Get the SMTK attribute being displayed.
smtk::attribute::ResourcePtr vtkAttributeMultiBlockSource::GetAttributeResource()
{
  return std::dynamic_pointer_cast<smtk::attribute::Resource>(this->GetResource());
}

/// Do nothing.
int vtkAttributeMultiBlockSource::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inInfo), vtkInformationVector* outInfo)
{
  auto output = vtkMultiBlockDataSet::GetData(outInfo, 0);
  if (!output)
  {
    vtkErrorMacro("No output dataset.");
    return 0;
  }

  auto resource = this->GetAttributeResource();
  if (!resource)
  {
    vtkErrorMacro("No input attribute.");
    return 0;
  }
  vtkAttributeMultiBlockSource::SetResourceId(output, resource->id());
  return 1;
}
