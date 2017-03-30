//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkPolygonArcOperator - convert polydata to polygon edge(s)
// .SECTION Description
// Operator to convert the line cells in the input polygon to multiple edge(s)
// in a polygon model.

#ifndef __smtk_polygon_vtkPolygonArcOperator_h
#define __smtk_polygon_vtkPolygonArcOperator_h

#include "smtk/bridge/polygon/vtk/Exports.h"
#include "smtk/extension/vtk/operators/vtkSMTKOperator.h"

class vtkPolyData;
class vtkContourRepresentation;

class VTKPOLYGONOPERATORSEXT_EXPORT vtkPolygonArcOperator : public vtkSMTKOperator
{
public:
  static vtkPolygonArcOperator* New();
  vtkTypeMacro(vtkPolygonArcOperator, vtkSMTKOperator);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Convert the passed-in polydata into polygon edge(s)
  virtual bool AbleToOperate();
  virtual smtk::model::OperatorResult Operate();

  //Description:
  //Get/Set the arc representation
  vtkGetMacro(ArcRepresentation, vtkContourRepresentation*);
  vtkSetMacro(ArcRepresentation, vtkContourRepresentation*);

protected:
  vtkPolygonArcOperator();
  virtual ~vtkPolygonArcOperator();

  vtkContourRepresentation* ArcRepresentation;

private:
  vtkPolygonArcOperator(const vtkPolygonArcOperator&); // Not implemented.
  void operator=(const vtkPolygonArcOperator&);        // Not implemented.
};

#endif
