//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMTKResourceGenerator.h"
#include "smtk/extension/paraview/server/vtkSMTKWrapper.h"

#include "smtk/resource/Manager.h"

#include "vtkCompositeDataIterator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

using namespace smtk;

vtkCxxSetObjectMacro(vtkSMTKResourceGenerator, Wrapper, vtkSMTKWrapper);

vtkSMTKResourceGenerator::vtkSMTKResourceGenerator()
{
  this->Wrapper = nullptr;
  this->SetNumberOfInputPorts(0);
}

vtkSMTKResourceGenerator::~vtkSMTKResourceGenerator()
{
  if (this->Wrapper)
  {
    this->DropResource();
  }
  this->SetWrapper(nullptr);
}

void vtkSMTKResourceGenerator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Wrapper: " << this->Wrapper << "\n";
}

smtk::resource::Resource::Ptr vtkSMTKResourceGenerator::GetResource() const
{
  return smtk::resource::Resource::Ptr();
}

void vtkSMTKResourceGenerator::DropResource()
{
  auto rsrc = this->GetResource();
  if (!rsrc)
  {
    return;
  }

  if (this->Wrapper != nullptr)
  {
    this->Wrapper->GetResourceManager()->remove(rsrc);
  }
}
