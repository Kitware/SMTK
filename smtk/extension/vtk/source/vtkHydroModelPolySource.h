//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkHydroModelPolySource - "Dummy" source so we can treat data as a source
// .SECTION Description
// The input Source data is shallow copied to the output

#ifndef __vtkHydroModelPolySource_h
#define __vtkHydroModelPolySource_h

#include "smtk/extension/vtk/source/Exports.h"
#include "vtkPolyDataAlgorithm.h"

class VTKSMTKSOURCEEXT_EXPORT vtkHydroModelPolySource : public vtkPolyDataAlgorithm
{
public:
  static vtkHydroModelPolySource* New();
  vtkTypeMacro(vtkHydroModelPolySource, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  void CopyData(vtkPolyData* source);
  vtkGetObjectMacro(Source, vtkPolyData);

protected:
  vtkHydroModelPolySource();
  ~vtkHydroModelPolySource() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  vtkPolyData* Source;

private:
  vtkHydroModelPolySource(const vtkHydroModelPolySource&); // Not implemented.
  void operator=(const vtkHydroModelPolySource&);          // Not implemented.
};

#endif
