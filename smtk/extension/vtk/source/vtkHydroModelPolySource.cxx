//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkHydroModelPolySource.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkHydroModelPolySource);

vtkHydroModelPolySource::vtkHydroModelPolySource()
{
  this->Source = vtkPolyData::New();
  this->SetNumberOfInputPorts(0);
}

vtkHydroModelPolySource::~vtkHydroModelPolySource()
{
  this->Source->Delete();
}

void vtkHydroModelPolySource::CopyData(vtkPolyData* source)
{
  this->Source->ShallowCopy(source);
  this->Modified();
}

int vtkHydroModelPolySource::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  // get the ouptut
  vtkPolyData* output = vtkPolyData::SafeDownCast(
    outputVector->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT()));

  // now move the input through to the output
  output->ShallowCopy(this->Source);
  return 1;
}

void vtkHydroModelPolySource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Source: " << this->Source << "\n";
}
