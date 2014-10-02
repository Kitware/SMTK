//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkDiscreteModelEntityGroup - An object to store a group of model entities.
// .SECTION Description
// An object that stores a group of model entities of the same EntityType.
// This class stores an association to those types as well as those
// types storing an association to this.  Note that a vtkModelEntity can
// belong to multiple vtkDiscreteModelEntityGroups (including none at all).

#ifndef __smtkcmb_vtkDiscreteModelEntityGroup_h
#define __smtkcmb_vtkDiscreteModelEntityGroup_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelEntity.h"


class vtkDiscreteModelEntity;

class VTKSMTKDISCRETEMODEL_EXPORT vtkDiscreteModelEntityGroup : public vtkModelEntity
{
public:
  vtkTypeMacro(vtkDiscreteModelEntityGroup,vtkModelEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  void AddModelEntity(vtkDiscreteModelEntity*);
  bool RemoveModelEntity(vtkDiscreteModelEntity*);

  // Description:
  // Returns the number of model entities of type this->EntityType
  // that is grouped by this object.
  int GetNumberOfModelEntities();

  // Description:
  // Returns an iterator over entities of type this->EntityType
  // that is grouped by this object.
  vtkModelItemIterator* NewModelEntityIterator();

  virtual int GetType();

  vtkSetMacro(EntityType, int);
  vtkGetMacro(EntityType, int);

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

protected:
  vtkDiscreteModelEntityGroup();
  virtual ~vtkDiscreteModelEntityGroup();
  static vtkDiscreteModelEntityGroup *New();
//BTX
  friend class vtkDiscreteModel;
//ETX

  virtual bool IsDestroyable();
  virtual bool Destroy();

  int EntityType;

private:
  vtkDiscreteModelEntityGroup(const vtkDiscreteModelEntityGroup&);  // Not implemented.
  void operator=(const vtkDiscreteModelEntityGroup&);  // Not implemented.
};

#endif

