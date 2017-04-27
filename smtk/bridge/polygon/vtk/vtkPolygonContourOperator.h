//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkPolygonContourOperator - convert contours to polygon edge(s)
// .SECTION Description
// Operator to convert the line cells in the input polygon to multiple edge(s)
// in a polygon model.

#ifndef __smtk_polygon_vtkPolygonContourOperator_h
#define __smtk_polygon_vtkPolygonContourOperator_h

#include "smtk/bridge/polygon/vtk/Exports.h"
#include "smtk/extension/vtk/operators/vtkSMTKOperator.h"

class vtkPolyData;

class VTKPOLYGONOPERATORSEXT_EXPORT vtkPolygonContourOperator : public vtkSMTKOperator
{
public:
  static vtkPolygonContourOperator* New();
  vtkTypeMacro(vtkPolygonContourOperator, vtkSMTKOperator);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Convert the passed-in polydata into polygon edge(s)
  virtual bool AbleToOperate();
  virtual smtk::model::OperatorResult Operate();

  //Description:
  //Get/Set the Contour Input
  vtkGetMacro(ContourInput, vtkPolyData*);
  vtkSetMacro(ContourInput, vtkPolyData*);

  // Description:
  // The bounds of the image where the contours are created from
  vtkSetVector6Macro(ImageBounds, double);
  vtkGetVector6Macro(ImageBounds, double);

protected:
  vtkPolygonContourOperator();
  virtual ~vtkPolygonContourOperator();

  vtkPolyData* ContourInput;
  double ImageBounds[6];

private:
  vtkPolygonContourOperator(const vtkPolygonContourOperator&); // Not implemented.
  void operator=(const vtkPolygonContourOperator&);            // Not implemented.
};

#endif
