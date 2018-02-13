//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSurfaceExtractorOperation - convert contours to polygon edge(s)
// .SECTION Description
// Operation to convert the line cells in the input polygon to multiple edge(s)
// in a polygon model.

#ifndef __smtk_polygon_vtkSurfaceExtractorOperation_h
#define __smtk_polygon_vtkSurfaceExtractorOperation_h

#include "smtk/extension/opencv/vtk/Exports.h"
#include "smtk/extension/vtk/operators/vtkSMTKOperation.h"

class vtkPolyData;

class VTKSMTKOPENCVEXT_EXPORT vtkSurfaceExtractorOperation : public vtkSMTKOperation
{
public:
  static vtkSurfaceExtractorOperation* New();
  vtkTypeMacro(vtkSurfaceExtractorOperation, vtkSMTKOperation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //Description:
  //Convert the passed-in polydata into polygon edge(s)
  bool AbleToOperate() override;
  smtk::operation::Operation::Result Operate() override;

protected:
  vtkSurfaceExtractorOperation();
  virtual ~vtkSurfaceExtractorOperation();

private:
  vtkSurfaceExtractorOperation(const vtkSurfaceExtractorOperation&); // Not implemented.
  void operator=(const vtkSurfaceExtractorOperation&);               // Not implemented.
};

#endif
