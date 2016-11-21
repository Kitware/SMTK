//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef vtkGrabCutFilter_h
#define vtkGrabCutFilter_h

#include "smtk/extension/vtk/filter/Exports.h" // For export macro

#include "vtkImageAlgorithm.h"

class VTKSMTKFILTEREXT_EXPORT vtkGrabCutFilter : public vtkImageAlgorithm
{
public:
  static vtkGrabCutFilter *New();
  vtkTypeMacro(vtkGrabCutFilter,vtkImageAlgorithm);

  vtkSetMacro(NumberOfIterations,int);
  vtkGetMacro(NumberOfIterations,int);

  vtkSetMacro(PotentialForegroundValue,int);
  vtkGetMacro(PotentialForegroundValue,int);

  vtkSetMacro(PotentialBackgroundValue,int);
  vtkGetMacro(PotentialBackgroundValue,int);

  vtkSetMacro(ForegroundValue,int);
  vtkGetMacro(ForegroundValue,int);

  vtkSetMacro(BackgroundValue,int);
  vtkGetMacro(BackgroundValue,int);

  void DoGrabCut()
  {
    RunGrabCuts = true;
    this->Modified();
  }

  ~vtkGrabCutFilter() override;

protected:

  int NumberOfIterations;
  int PotentialForegroundValue;
  int PotentialBackgroundValue;
  int ForegroundValue;
  int BackgroundValue;

  vtkGrabCutFilter();

  int FillOutputPortInformation(int port, vtkInformation *info) override;

  int RequestData(vtkInformation *, vtkInformationVector **,
                  vtkInformationVector *) override;

private:
  vtkGrabCutFilter(const vtkGrabCutFilter&);  // Not implemented.
  void operator=(const vtkGrabCutFilter&);  // Not implemented.
  bool RunGrabCuts;
  class InternalData;
  InternalData * internal;
};

#endif
