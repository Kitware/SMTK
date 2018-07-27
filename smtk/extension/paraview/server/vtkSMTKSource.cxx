//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMTKSource.h"

#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include "vtkCompositeDataIterator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

#include <algorithm>

using namespace smtk;

vtkStandardNewMacro(vtkSMTKSource);

vtkSMTKSource::vtkSMTKSource()
  : ResourceGenerator(nullptr)
{
  this->SetNumberOfInputPorts(0);
}

vtkSMTKSource::~vtkSMTKSource()
{
}

void vtkSMTKSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Generator: " << this->ResourceGenerator << "\n";
}

vtkMTimeType vtkSMTKSource::GetMTime()
{
  // Access the modification time of this class ignoring the resource generator.
  vtkMTimeType mTime = this->vtkObject::GetMTime();

  // Access the modification time of the resource generator.
  vtkMTimeType resourceGenerator_mTime =
    (this->ResourceGenerator ? this->ResourceGenerator->GetMTime() : mTime);

  // The outward-facing modification time of this class is the latest of these
  // two times.
  return std::max({ mTime, resourceGenerator_mTime });
}

int vtkSMTKSource::FillOutputPortInformation(int port, vtkInformation* info)
{
  // We must have a resource generator to query for output port information.
  if (this->ResourceGenerator == nullptr)
  {
    vtkDebugMacro("Resource generator is not set.");
    return 0;
  }

  this->SetNumberOfOutputPorts(this->ResourceGenerator->GetNumberOfOutputPorts());

  vtkInformation* rinfo = this->ResourceGenerator->GetOutputPortInformation(port);
  info->CopyEntry(rinfo, vtkDataObject::DATA_TYPE_NAME());
  return 1;
}

int vtkSMTKSource::RequestData(
  vtkInformation* request, vtkInformationVector** inInfo, vtkInformationVector* outInfo)
{
  // We must have a resource generator to operate
  if (this->ResourceGenerator == nullptr)
  {
    vtkDebugMacro("Resource generator is not set.");
    return 0;
  }

  // If the resource generator has been updated after this class has, execute
  // the resource generator.
  if (this->ResourceGenerator->GetMTime() > this->vtkObject::GetMTime())
  {
    return this->ResourceGenerator->ProcessRequest(request, inInfo, outInfo);
  }

  // Otherwise, access the resource generator's smtk -> vtkMultiBlockDataSet
  // converter and trigger it to rerender the resource.
  vtkAlgorithm* converter = this->ResourceGenerator->GetConverter();
  converter->Modified();
  converter->Update();

  // Grab the output from the converter and assign it as the output for this
  // method.
  for (int i = 0; i < this->ResourceGenerator->GetNumberOfOutputPorts(); i++)
  {
    vtkMultiBlockDataSet::GetData(outInfo, i)->ShallowCopy(converter->GetOutputDataObject(i));
  }

  return 1;
}
