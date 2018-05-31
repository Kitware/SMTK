//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/filter/vtkImageDual.h"

#include "vtkObjectFactory.h"

#include "vtkCellData.h"
#include "vtkDataObject.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkNew.h"
#include "vtkPointData.h"

vtkStandardNewMacro(vtkImageDual);

vtkImageDual::vtkImageDual()
{
}

vtkImageDual::~vtkImageDual()
{
}

int vtkImageDual::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkImageData* input = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData* output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  int extent[6];
  double spacing[3];
  double origin[3];
  input->GetExtent(extent);
  input->GetSpacing(spacing);
  input->GetOrigin(origin);

  for (int i = 0; i < 3; i++)
  {
    --extent[2 * i + 1];
    if (extent[2 * i] > extent[2 * i + 1])
    {
      extent[2 * i + 1] = extent[2 * i];
    }
  }

  output->SetExtent(extent);
  output->SetSpacing(spacing);
  output->SetOrigin(origin);
  output->GetPointData()->CopyAllOn();

  output->GetPointData()->PassData(input->GetCellData());

  return 1;
}
