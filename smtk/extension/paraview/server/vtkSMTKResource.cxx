//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMTKResource.h"
#include "smtk/extension/paraview/server/vtkSMTKWrapper.h"

#include "smtk/extension/vtk/source/SourceGenerator.h"
#include "smtk/extension/vtk/source/vtkResourceMultiBlockSource.h"

#include "smtk/resource/Manager.h"

#include "vtkCompositeDataIterator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

#include <algorithm>

using namespace smtk;

vtkStandardNewMacro(vtkSMTKResource);
vtkCxxSetObjectMacro(vtkSMTKResource, Wrapper, vtkSMTKWrapper);

vtkSMTKResource::vtkSMTKResource()
  : Converter(nullptr)
{
  this->SetNumberOfInputPorts(0);
}

vtkSMTKResource::~vtkSMTKResource()
{
  this->SetWrapper(nullptr);
  this->SetResource(nullptr);
}

void vtkSMTKResource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Resource: " << (this->GetResource() ? this->GetResource()->location() : "(null)")
     << "\n";
}

void vtkSMTKResource::SetResourceById(const char* resourceIdStr)
{
  vtkDebugMacro(
    << this->GetClassName() << " (" << this << "): setting resource to "
    << (resourceIdStr ? resourceIdStr : "(null)"));

  if (resourceIdStr == nullptr || this->Wrapper == nullptr)
  {
    return;
  }

  smtk::common::UUID resourceId(resourceIdStr);

  auto resource = this->Wrapper->GetResourceManager()->get(resourceId);
  this->SetResource(resource);
}

void vtkSMTKResource::SetResource(const smtk::resource::ResourcePtr& resource)
{
  if (this->Resource.lock() != resource)
  {
    this->Resource = resource;
    this->Modified();
  }
}

void vtkSMTKResource::DropResource()
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

vtkAlgorithm* vtkSMTKResource::GetConverter()
{
  if (this->Converter == nullptr && this->GetResource() != nullptr)
  {
    this->Converter = smtk::extension::vtk::source::Generator()(this->GetResource());

    if (!this->Converter)
    {
      auto* source = vtkResourceMultiBlockSource::New();
      source->SetResource(this->GetResource());
      this->Converter = source;
    }
  }
  return this->Converter;
}

int vtkSMTKResource::RequestData(
  vtkInformation* /*unused*/,
  vtkInformationVector** /*unused*/,
  vtkInformationVector* outInfo)
{
  // Access the converter, constructing and initializing it if necessary.
  auto* converter = this->GetConverter();

  // We must have a resource generator to operate
  if (this->GetConverter() == nullptr)
  {
    vtkDebugMacro("Resource is not set.");
    return 0;
  }

  // Trigger the resource's smtk -> vtkAlgorithm converter to rerender the resource.
  converter->Modified();
  converter->Update();

  // Grab the output from the converter and assign it as the output for this
  // method.
  for (int i = 0; i < converter->GetNumberOfOutputPorts(); i++)
  {
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 20221201)
    vtkMultiBlockDataSet::GetData(outInfo, i)
      ->CompositeShallowCopy(vtkMultiBlockDataSet::SafeDownCast(converter->GetOutputDataObject(i)));
#else
    vtkMultiBlockDataSet::GetData(outInfo, i)->ShallowCopy(converter->GetOutputDataObject(i));
#endif
  }

  return 1;
}
