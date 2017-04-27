//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/filter/vtkImageSpacingFlip.h"

#include "vtkObjectFactory.h"

#include "vtkDataObject.h"
#include "vtkImageData.h"
#include "vtkImageFlip.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkNew.h"

vtkStandardNewMacro(vtkImageSpacingFlip);

vtkImageSpacingFlip::vtkImageSpacingFlip()
{
}

vtkImageSpacingFlip::~vtkImageSpacingFlip()
{
}

int vtkImageSpacingFlip::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkImageData* input = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData* output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  double* spacing = input->GetSpacing();
  double origin[3];
  input->GetOrigin(origin);

  if (spacing[0] >= 0 && spacing[1] >= 0 && spacing[2] >= 0)
  {
    output->ShallowCopy(input);
  }
  else
  {
    vtkNew<vtkImageFlip> flipper;
    if (spacing[0] < 0 && spacing[1] >= 0 && spacing[2] >= 0)
    {
      flipper->SetFilteredAxis(0);
      origin[0] = origin[0] + spacing[0] * input->GetDimensions()[0];
    }
    else if (spacing[1] < 0 && spacing[0] >= 0 && spacing[2] >= 0)
    {
      flipper->SetFilteredAxis(1);
      origin[1] = origin[1] + spacing[1] * input->GetDimensions()[1];
    }
    else if (spacing[2] < 0 && spacing[0] >= 0 && spacing[1] >= 0)
    {
      flipper->SetFilteredAxis(2);
      origin[2] = origin[2] + spacing[2] * input->GetDimensions()[2];
    }
    else
    {
      //Error message for now
      vtkErrorMacro(<< " Currently only supports on dimension being negative.");
      output->DeepCopy(input);
      return 1;
    }
    flipper->FlipAboutOriginOff();
    flipper->SetInputData(input);
    flipper->Update();
    output->DeepCopy(flipper->GetOutput());
    output->SetSpacing(fabs(spacing[0]), fabs(spacing[1]), fabs(spacing[2]));
    output->SetOrigin(origin);
  }

  return 1;
}
