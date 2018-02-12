//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkPolygonArcOperation - convert polydata to polygon edge(s)
// .SECTION Description
// Operation to convert the line cells in the input polygon to multiple edge(s)
// in a polygon model.

#ifndef __smtk_polygon_vtkPolygonArcOperation_h
#define __smtk_polygon_vtkPolygonArcOperation_h

#include "smtk/bridge/polygon/vtk/Exports.h"
#include "smtk/extension/vtk/operators/vtkSMTKOperation.h"

class vtkPolyData;
class vtkContourRepresentation;

class VTKPOLYGONOPERATIONSEXT_EXPORT vtkPolygonArcOperation : public vtkSMTKOperation
{
public:
  static vtkPolygonArcOperation* New();
  vtkTypeMacro(vtkPolygonArcOperation, vtkSMTKOperation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //Description:
  //Convert the passed-in polydata into polygon edge(s)
  bool AbleToOperate() override;
  smtk::operation::Operation::Result Operate() override;

  //Description:
  //Get/Set the arc representation
  vtkGetObjectMacro(ArcRepresentation, vtkContourRepresentation);
  virtual void SetArcRepresentation(vtkContourRepresentation*);

protected:
  vtkPolygonArcOperation();
  virtual ~vtkPolygonArcOperation();

  vtkContourRepresentation* ArcRepresentation;

private:
  vtkPolygonArcOperation(const vtkPolygonArcOperation&); // Not implemented.
  void operator=(const vtkPolygonArcOperation&);         // Not implemented.
};

#endif
