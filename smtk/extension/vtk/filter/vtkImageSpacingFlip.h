//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkImageSpacingFlip - Flips the image so spacing is positive
// .SECTION Description
// Flips the image so spacing is positive.

#ifndef vtkImageSpacingFlip_h
#define vtkImageSpacingFlip_h

#include "smtk/extension/vtk/filter/Exports.h" // For export macro

#include "vtkImageAlgorithm.h"

class VTKSMTKFILTEREXT_EXPORT vtkImageSpacingFlip : public vtkImageAlgorithm
{
public:
  static vtkImageSpacingFlip* New();
  vtkTypeMacro(vtkImageSpacingFlip, vtkImageAlgorithm);

protected:
  vtkImageSpacingFlip();
  ~vtkImageSpacingFlip();

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkImageSpacingFlip(const vtkImageSpacingFlip&); // Not implemented.
  void operator=(const vtkImageSpacingFlip&);      // Not implemented.
};

#endif
