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

int vtkSMTKResourceGenerator::RequestData(
  vtkInformation* request,
  vtkInformationVector** inInfo,
  vtkInformationVector* outInfo)
{
  this->SetResource(this->GenerateResource());

  return vtkSMTKResource::RequestData(request, inInfo, outInfo);
}
