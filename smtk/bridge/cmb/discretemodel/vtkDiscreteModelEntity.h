//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkDiscreteModelEntity -
// .SECTION Description

#ifndef __smtkcmb_vtkDiscreteModelEntity_h
#define __smtkcmb_vtkDiscreteModelEntity_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkObject.h"


class vtkDiscreteModelEntityGroup;
class vtkModelEntity;
class vtkModelItemIterator;

class VTKSMTKDISCRETEMODEL_EXPORT vtkDiscreteModelEntity
{
public:
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get a pointer to this object that is a vtkModelEntity.
  virtual vtkModelEntity* GetThisModelEntity()=0;

  // Description:
  // Given a vtkModelEntity, return a vtkDiscreteModelEntity if
  // it is a vtkDiscreteModelEntity (for now vtkDiscreteModelRegion,
  // vtkDiscreteModelFace, or vtkDiscreteModelEdge).
  static vtkDiscreteModelEntity* GetThisDiscreteModelEntity(vtkModelEntity*);

  // Description:
  // Get information about the model entity groups associated
  // with this object.
  int GetNumberOfModelEntityGroups();
  vtkModelItemIterator* NewModelEntityGroupIterator();

protected:
  vtkDiscreteModelEntity();
  virtual ~vtkDiscreteModelEntity();

  void CopyModelEntityGroups(vtkDiscreteModelEntity* sourceEntity);
  void RemoveAllModelEntityGroups();

private:
  vtkDiscreteModelEntity(const vtkDiscreteModelEntity&);  // Not implemented.
  void operator=(const vtkDiscreteModelEntity&);  // Not implemented.
};

#endif

