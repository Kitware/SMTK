//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkImageDual - Construct the dual of a vtkImageData
// .SECTION Description
// vtkGDALRasterReader has switched from storing its data as point data to
// cell data. Traditional vtkImageData routines expect data as point data.
// We therefore must construct a dual graph to maintain VTK's paradigm.

#ifndef vtkImageDual_h
#define vtkImageDual_h

#include "smtk/extension/vtk/filter/vtkSMTKFilterExtModule.h" // For export macro

#include "vtkImageAlgorithm.h"

class VTKSMTKFILTEREXT_EXPORT vtkImageDual : public vtkImageAlgorithm
{
public:
  static vtkImageDual* New();
  vtkTypeMacro(vtkImageDual, vtkImageAlgorithm);

protected:
  vtkImageDual();
  ~vtkImageDual() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkImageDual(const vtkImageDual&);   // Not implemented.
  void operator=(const vtkImageDual&); // Not implemented.
};

#endif
