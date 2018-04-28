//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMTKResourceReader.h"
#include "smtk/extension/paraview/server/vtkSMTKWrapper.h"

#include "smtk/resource/Manager.h"

#include "vtkCompositeDataIterator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

using namespace smtk;

vtkCxxSetObjectMacro(vtkSMTKResourceReader, Wrapper, vtkSMTKWrapper);

vtkSMTKResourceReader::vtkSMTKResourceReader()
{
  this->FileName = nullptr;
  this->Wrapper = nullptr;
  this->SetNumberOfInputPorts(0);
}

vtkSMTKResourceReader::~vtkSMTKResourceReader()
{
  if (this->Wrapper)
  {
    this->DropResource();
  }
  this->SetWrapper(nullptr);
  this->SetFileName(nullptr);
}

void vtkSMTKResourceReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
  os << indent << "Wrapper: " << this->Wrapper << "\n";
}

smtk::resource::Resource::Ptr vtkSMTKResourceReader::GetResource() const
{
  return smtk::resource::Resource::Ptr();
}

void vtkSMTKResourceReader::DropResource()
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
