//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef vtkImageClassFilter_h
#define vtkImageClassFilter_h

#include "smtk/extension/vtk/filter/Exports.h" // For export macro

#include "vtkImageAlgorithm.h"

class VTKSMTKFILTEREXT_EXPORT vtkImageClassFilter : public vtkImageAlgorithm
{
public:
  static vtkImageClassFilter* New();
  vtkTypeMacro(vtkImageClassFilter, vtkImageAlgorithm);

  vtkSetMacro(ForegroundValue, int);
  vtkGetMacro(ForegroundValue, int);

  vtkSetMacro(BackgroundValue, int);
  vtkGetMacro(BackgroundValue, int);

  vtkSetMacro(MinFGSize, double);
  vtkGetMacro(MinFGSize, double);

  vtkSetMacro(MinBGSize, double);
  vtkGetMacro(MinBGSize, double);

  ~vtkImageClassFilter() override;

protected:
  int ForegroundValue;
  int BackgroundValue;

  double MinFGSize;
  double MinBGSize;

  vtkImageClassFilter();

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkImageClassFilter(const vtkImageClassFilter&); // Not implemented.
  void operator=(const vtkImageClassFilter&);      // Not implemented.
};

#endif
