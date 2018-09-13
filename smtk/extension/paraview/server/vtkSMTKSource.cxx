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
  : VTKResource(nullptr)
{
  this->SetNumberOfInputPorts(0);

  // Set the number of output ports to be the maximum of the number of output
  // ports for the available converters (in this case, vtkModelMultiBlockSource).
  this->SetNumberOfOutputPorts(vtkModelMultiBlockSource::NUMBER_OF_OUTPUT_PORTS);
}

vtkSMTKSource::~vtkSMTKSource()
{
}

void vtkSMTKSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "VTKResource: " << this->VTKResource << "\n";
}

vtkMTimeType vtkSMTKSource::GetMTime()
{
  // Access the modification time of this class ignoring the resource.
  vtkMTimeType mTime = this->vtkObject::GetMTime();

  // Access the modification time of the resource.
  vtkMTimeType resource_mTime = (this->VTKResource ? this->VTKResource->GetMTime() : mTime);

  // The outward-facing modification time of this class is the latest of these
  // two times.
  return std::max({ mTime, resource_mTime });
}

int vtkSMTKSource::FillOutputPortInformation(int port, vtkInformation* info)
{
  // We must have a resource to query for output port information.
  if (this->VTKResource == nullptr)
  {
    vtkDebugMacro("Resource is not set.");
    return 0;
  }

  if (this->VTKResource->GetNumberOfOutputPorts() > port)
  {
    vtkInformation* rinfo = this->VTKResource->GetOutputPortInformation(port);
    info->CopyEntry(rinfo, vtkDataObject::DATA_TYPE_NAME());
  }
  else
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkMultiBlockDataSet");
  }
  return 1;
}

int vtkSMTKSource::RequestData(
  vtkInformation* request, vtkInformationVector** inInfo, vtkInformationVector* outInfo)
{
  // We must have a resource to operate
  if (this->VTKResource == nullptr)
  {
    vtkDebugMacro("Resource is not set.");
    return 0;
  }

  // If the resource has been updated after this class has, execute the resource.
  if (this->VTKResource->GetMTime() > this->vtkObject::GetMTime())
  {
    return this->VTKResource->ProcessRequest(request, inInfo, outInfo);
  }

  // Otherwise, access the resource generator's smtk -> vtkMultiBlockDataSet
  // converter and trigger it to rerender the resource.
  vtkAlgorithm* converter = this->VTKResource->GetConverter();
  converter->Modified();
  converter->Update();

  // Grab the output from the converter and assign it as the output for this
  // method.
  for (int i = 0; i < this->VTKResource->GetNumberOfOutputPorts(); i++)
  {
    if (this->VTKResource->GetNumberOfOutputPorts() > i)
    {
      vtkMultiBlockDataSet::GetData(outInfo, i)->ShallowCopy(converter->GetOutputDataObject(i));
    }
  }

  return 1;
}
