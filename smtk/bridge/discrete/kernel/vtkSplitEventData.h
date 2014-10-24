//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkSplitEventData - Information for a split event.
// .SECTION Description
// This class stores information for a split event that has occurred.
// The SourceEntity was the entity that was split and the
// CreatedModelEntityIds are the Ids of the objects created.
// Note that a model entity is always split into 2 entities of the same
// dimension.  If there are multiple splits as part of an operation
// (e.g. split based on angle) then the event will be triggered
// multiple times.

#ifndef __smtkdiscrete_vtkSplitEventData_h
#define __smtkdiscrete_vtkSplitEventData_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro

#include "vtkObject.h"

class vtkIdList;
class vtkDiscreteModelGeometricEntity;

class VTKSMTKDISCRETEMODEL_EXPORT vtkSplitEventData : public vtkObject
{
public:
  static vtkSplitEventData * New();
  vtkTypeMacro(vtkSplitEventData,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get the SourceEntity
  vtkGetMacro(SourceEntity, vtkDiscreteModelGeometricEntity*);
  vtkSetMacro(SourceEntity, vtkDiscreteModelGeometricEntity*);

  // Description:
  // Set/get the list of created model entity ids.  Values
  // can be for different model entity types.
  vtkGetMacro(CreatedModelEntityIds, vtkIdList*);
  void SetCreatedModelEntityIds(vtkIdList*);

protected:
  vtkSplitEventData();
  virtual ~vtkSplitEventData();

private:
  vtkDiscreteModelGeometricEntity* SourceEntity;
  vtkIdList* CreatedModelEntityIds;

  vtkSplitEventData(const vtkSplitEventData&);  // Not implemented.
  void operator=(const vtkSplitEventData&);  // Not implemented.
};

#endif
