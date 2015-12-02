//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkMergeEventData - Information for a merge event.
// .SECTION Description
// This class stores information for a merge event that is about to take
// place.  The SourceEntity is merged into the TargetEntity and the
// LowerDimensionalIds is the list of ids that are only on the boundary
// of the SourceEntity and TargetEntity and will be destroyed during
// the merge operation.

#ifndef __smtkdiscrete_vtkMergeEventData_h
#define __smtkdiscrete_vtkMergeEventData_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkObject.h"


class vtkIdTypeArray;
class vtkDiscreteModelGeometricEntity;

class VTKSMTKDISCRETEMODEL_EXPORT vtkMergeEventData : public vtkObject
{
public:
  static vtkMergeEventData * New();
  vtkTypeMacro(vtkMergeEventData,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get the SourceEntity
  vtkGetMacro(SourceEntity, vtkDiscreteModelGeometricEntity*);
  vtkSetMacro(SourceEntity, vtkDiscreteModelGeometricEntity*);

  // Description:
  // Set/get the TargetEntity
  vtkGetMacro(TargetEntity, vtkDiscreteModelGeometricEntity*);
  vtkSetMacro(TargetEntity, vtkDiscreteModelGeometricEntity*);

  // Description:
  // Set/get the LowerDimensionalIds
  vtkGetMacro(LowerDimensionalIds, vtkIdTypeArray*);
  void SetLowerDimensionalIds(vtkIdTypeArray* lowerDimensionalIds);

protected:
  vtkMergeEventData();
  virtual ~vtkMergeEventData();

private:
  vtkDiscreteModelGeometricEntity* SourceEntity;
  vtkDiscreteModelGeometricEntity* TargetEntity;
  vtkIdTypeArray* LowerDimensionalIds;

  vtkMergeEventData(const vtkMergeEventData&);  // Not implemented.
  void operator=(const vtkMergeEventData&);  // Not implemented.
};

#endif
