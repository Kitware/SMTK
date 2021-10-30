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

#ifndef smtk_polygon_vtkPolygonArcOperation_h
#define smtk_polygon_vtkPolygonArcOperation_h

#include "smtk/extension/vtk/operators/vtkSMTKOperation.h"
#include "smtk/session/polygon/vtk/vtkPolygonOperationsExtModule.h"

class vtkPolyData;
class vtkContourRepresentation;

class VTKPOLYGONOPERATIONSEXT_EXPORT vtkPolygonArcOperation : public vtkSMTKOperation
{
public:
  static vtkPolygonArcOperation* New();
  vtkTypeMacro(vtkPolygonArcOperation, vtkSMTKOperation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkPolygonArcOperation(const vtkPolygonArcOperation&) = delete;
  vtkPolygonArcOperation& operator=(const vtkPolygonArcOperation&) = delete;

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
  ~vtkPolygonArcOperation() override;

  vtkContourRepresentation* ArcRepresentation;
};

#endif
