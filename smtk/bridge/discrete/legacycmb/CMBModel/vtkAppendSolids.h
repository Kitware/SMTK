//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkAppendSolids - filter appends polydata inputs and adds region specifier
// .SECTION Description
// This filter is basically a glorified vtkAppendPolyData filter that also
// sets the "Region" value of each input to a different value (starting at 0).
// The output is also cleaned (if there is more than 1 input).
// .SECTION See Also

#ifndef __vtkAppendSolids_h
#define __vtkAppendSolids_h

#include "cmbSystemConfig.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class vtkPolyData;

class VTKCMBDISCRETEMODEL_EXPORT vtkAppendSolids : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkAppendSolids, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkAppendSolids* New();

  // Description:
  // Set 2nd input input to the filter (required)
  void AddInputData(vtkPolyData* input);

  vtkSetStringMacro(RegionArrayName);
  vtkGetStringMacro(RegionArrayName);

protected:
  vtkAppendSolids();
  ~vtkAppendSolids();

  // Usual data generation method
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);
  virtual int FillInputPortInformation(int, vtkInformation*);

private:
  vtkAppendSolids(const vtkAppendSolids&); // Not implemented.
  void operator=(const vtkAppendSolids&);  // Not implemented.

  char* RegionArrayName;
};

#endif
