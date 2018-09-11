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

#ifndef __smtkdiscrete_vtkDiscreteModelEntityGroup_h
#define __smtkdiscrete_vtkDiscreteModelEntityGroup_h

#include "smtk/session/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelEntity.h"

class vtkDiscreteModelEntity;

class VTKSMTKDISCRETEMODEL_EXPORT vtkDiscreteModelEntityGroup : public vtkModelEntity
{
public:
  vtkTypeMacro(vtkDiscreteModelEntityGroup, vtkModelEntity);
  void PrintSelf(ostream& os, vtkIndent indent) override;

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

  int GetType() override;

  vtkSetMacro(EntityType, int);
  vtkGetMacro(EntityType, int);

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  void Serialize(vtkSerializer* ser) override;

protected:
  vtkDiscreteModelEntityGroup();
  ~vtkDiscreteModelEntityGroup() override;
  static vtkDiscreteModelEntityGroup* New();

  friend class vtkDiscreteModel;

  virtual bool IsDestroyable();
  bool Destroy() override;

  int EntityType;

private:
  vtkDiscreteModelEntityGroup(const vtkDiscreteModelEntityGroup&); // Not implemented.
  void operator=(const vtkDiscreteModelEntityGroup&);              // Not implemented.
};

#endif
