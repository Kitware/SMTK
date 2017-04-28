//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSurfaceExtractorOperator - convert contours to polygon edge(s)
// .SECTION Description
// Operator to convert the line cells in the input polygon to multiple edge(s)
// in a polygon model.

#ifndef __smtk_polygon_vtkSurfaceExtractorOperator_h
#define __smtk_polygon_vtkSurfaceExtractorOperator_h

#include "smtk/extension/opencv/vtk/Exports.h"
#include "smtk/extension/vtk/operators/vtkSMTKOperator.h"

class vtkPolyData;

class VTKSMTKOPENCVEXT_EXPORT vtkSurfaceExtractorOperator : public vtkSMTKOperator
{
public:
  static vtkSurfaceExtractorOperator* New();
  vtkTypeMacro(vtkSurfaceExtractorOperator, vtkSMTKOperator);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Convert the passed-in polydata into polygon edge(s)
  virtual bool AbleToOperate();
  virtual smtk::model::OperatorResult Operate();

protected:
  vtkSurfaceExtractorOperator();
  virtual ~vtkSurfaceExtractorOperator();

private:
  vtkSurfaceExtractorOperator(const vtkSurfaceExtractorOperator&); // Not implemented.
  void operator=(const vtkSurfaceExtractorOperator&);              // Not implemented.
};

#endif
