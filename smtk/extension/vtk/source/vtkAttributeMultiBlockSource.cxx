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

  os << indent << "Resource: " << this->Resource.get() << "\n";
}

/// Set the SMTK model to be displayed.
void vtkAttributeMultiBlockSource::SetResource(smtk::attribute::ResourcePtr attr)
{
  if (this->Resource == attr)
  {
    return;
  }
  this->Resource = attr;
  this->Modified();
}

/// Get the SMTK attribute being displayed.
smtk::attribute::ResourcePtr vtkAttributeMultiBlockSource::GetResource()
{
  return this->Resource;
}

/// Do nothing.
int vtkAttributeMultiBlockSource::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inInfo), vtkInformationVector* vtkNotUsed(outInfo))
{
  return 1;
}
