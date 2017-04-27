//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkPolygonArcInfo -
// .SECTION Description
// .SECTION Caveats This class only works in built-in mode

#ifndef __smtk_polygon_vtkPolygonArcInfo_h
#define __smtk_polygon_vtkPolygonArcInfo_h

#include "smtk/bridge/polygon/qt/Exports.h"
#include "vtkPVInformation.h"

class SMTKPOLYGONQTEXT_EXPORT vtkPolygonArcInfo : public vtkPVInformation
{
public:
  static vtkPolygonArcInfo* New();
  vtkTypeMacro(vtkPolygonArcInfo, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Transfer information about a single object into this object.
  virtual void CopyFromObject(vtkObject*);

  // Description:
  // Manage a serialized version of the information.
  virtual void CopyToStream(vtkClientServerStream*);
  virtual void CopyFromStream(const vtkClientServerStream*);

  //Description:
  //Returns if the this arc is a closed loop
  bool IsClosedLoop() { return ClosedLoop; }

  //Description:
  //Returns the total number of points which is end nodes + internal points
  vtkIdType GetNumberOfPoints() { return NumberOfPoints; };

  // Description:
  // Set/Get the block index to extract information from
  vtkSetMacro(BlockIndex, vtkIdType);
  vtkGetMacro(BlockIndex, vtkIdType);

  // Description:
  // Set/Get the SelectedPointId to querry point coordinates
  vtkSetMacro(SelectedPointId, vtkIdType);
  vtkGetMacro(SelectedPointId, vtkIdType);

  // Description:
  // Get Model entity ID that this information is related.
  vtkGetStringMacro(ModelEntityID);

  // Get point cooridnates for the SelectedPointId of the block.
  vtkGetVector3Macro(SelectedPointCoordinates, double);

protected:
  vtkPolygonArcInfo();
  ~vtkPolygonArcInfo();

  vtkSetStringMacro(ModelEntityID);
  vtkSetVector3Macro(SelectedPointCoordinates, double);

  bool ClosedLoop;
  vtkIdType BlockIndex;
  vtkIdType SelectedPointId;
  vtkIdType NumberOfPoints;
  double SelectedPointCoordinates[3];
  char* ModelEntityID;

private:
  vtkPolygonArcInfo(const vtkPolygonArcInfo&); // Not implemented
  void operator=(const vtkPolygonArcInfo&);    // Not implemented
};

#endif
