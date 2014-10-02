//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkPVModelGeometryInformation - Light object for holding
// geometry information about a model face object.
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVModelGeometryInformation_h
#define __vtkPVModelGeometryInformation_h

#include "vtkPVInformation.h"
#include "cmbSystemConfig.h"
#include <string>
#include <map>

class vtkIdTypeArray;

class VTK_EXPORT vtkPVModelGeometryInformation : public vtkPVInformation
{
public:
  static vtkPVModelGeometryInformation* New();
  vtkTypeMacro(vtkPVModelGeometryInformation, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Transfer information about a single object into this object.
  virtual void CopyFromObject(vtkObject*);

  // Description:
  // Get the number of points or cells for the model or a model entity.
  vtkGetMacro(NumberOfPoints, int);
  vtkGetMacro(NumberOfCells, int);

  // Description:
  // Get the bounds for the model or a geometric model entity as
  // (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
  void GetBounds(double bounds[6]);
  double *GetBounds();

  virtual int GetMasterCellId(unsigned int flatidx, int idx);
  virtual vtkIdType GetModelEntityId(unsigned int flatidx);

  // Description:
  // Manage a serialized version of the information.
  virtual void CopyToStream(vtkClientServerStream*);
  virtual void CopyFromStream(const vtkClientServerStream*);

  //BTX
protected:
  vtkPVModelGeometryInformation();
  ~vtkPVModelGeometryInformation();

  // Data information collected from remote processes.
  int    NumberOfPoints;
  int    NumberOfCells;
  double Bounds[6];

  std::map<int, vtkIdTypeArray*> CellIdsMap;
  std::map<int, vtkIdType> EnityIdsMap;

private:

  vtkPVModelGeometryInformation(const vtkPVModelGeometryInformation&); // Not implemented
  void operator=(const vtkPVModelGeometryInformation&); // Not implemented
  //ETX
};

#endif
