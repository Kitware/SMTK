//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef vtkWatershedFilter_h
#define vtkWatershedFilter_h

#include "smtk/extension/vtk/filter/Exports.h" // For export macro

#include "vtkImageAlgorithm.h"

class VTKSMTKFILTEREXT_EXPORT vtkWatershedFilter : public vtkImageAlgorithm
{
public:
  static vtkWatershedFilter* New();
  vtkTypeMacro(vtkWatershedFilter, vtkImageAlgorithm);

  vtkSetMacro(ForegroundValue, int);
  vtkGetMacro(ForegroundValue, int);

  vtkSetMacro(BackgroundValue, int);
  vtkGetMacro(BackgroundValue, int);

  vtkSetMacro(UnlabeledValue, int);
  vtkGetMacro(UnlabeledValue, int);

protected:
  int ForegroundValue;
  int BackgroundValue;
  int UnlabeledValue;

  vtkWatershedFilter();

  int FillOutputPortInformation(int port, vtkInformation* info) override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkWatershedFilter(const vtkWatershedFilter&); // Not implemented.
  void operator=(const vtkWatershedFilter&);     // Not implemented.
};

#endif
