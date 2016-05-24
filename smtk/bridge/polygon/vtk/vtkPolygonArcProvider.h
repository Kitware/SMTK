//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArc - vtkDataObject that represent a single arc
// .SECTION Description
// An arc is represented by a line with 1 or 2 end nodes
// and a collection of internal points.
// Each arc has a unique Id

#ifndef __smtk_polygon_vtkPolygonArcProvider_h
#define __smtk_polygon_vtkPolygonArcProvider_h

#include "smtk/bridge/polygon/vtk/Exports.h"
#include "vtkPolyDataAlgorithm.h"

class vtkPolyData;

class VTKPOLYGONOPERATORSEXT_EXPORT vtkPolygonArcProvider : public vtkPolyDataAlgorithm
{
public:
  static vtkPolygonArcProvider *New();
  vtkTypeMacro(vtkPolygonArcProvider,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Select the block index to be extracted.  The filter will iterate through
  // the leves of the dataset until it reaches the indicated leaf block.
  vtkSetMacro(BlockIndex, vtkIdType);
  vtkGetMacro(BlockIndex, vtkIdType);

protected:
  vtkPolygonArcProvider();
  ~vtkPolygonArcProvider();

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

  vtkIdType BlockIndex;

private:
  vtkPolygonArcProvider(const vtkPolygonArcProvider&);  // Not implemented.
  void operator=(const vtkPolygonArcProvider&);  // Not implemented.
};

#endif
