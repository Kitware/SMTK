//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelEntityGroupOperationBase - Modify a model entity group.
// .SECTION Description
// Operation to change information about a model entity group.  As this
// derives from vtkModelEntityOperation, you can use
// this to also modify RGBA,
// user name and/or visibility of a model entity group in addition to
// adding and/or removing model entities that are associated with
// a model entity group.

#ifndef __smtkdiscrete_vtkModelEntityGroupOperationBase_h
#define __smtkdiscrete_vtkModelEntityGroupOperationBase_h

#include "smtk/session/discrete/Exports.h" // For export macro
#include "vtkModelEntityOperationBase.h"

class vtkDiscreteModelEntityGroup;
class vtkDiscreteModel;
class vtkDiscreteModelEntity;
class vtkIdList;
class vtkModelEntity;

class SMTKDISCRETESESSION_EXPORT vtkModelEntityGroupOperationBase
  : public vtkModelEntityOperationBase
{
public:
  static vtkModelEntityGroupOperationBase* New();
  vtkTypeMacro(vtkModelEntityGroupOperationBase, vtkModelEntityOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set the model entity type to vtkDiscreteModelEntityGroupType to override
  // the superclass virtual function.
  void SetItemType(int itemType) override;

  // Description:
  // Get the model entity group from Model.
  vtkDiscreteModelEntityGroup* GetModelEntityGroup(vtkDiscreteModel* Model);

  // Description:
  // Add a vtkDiscreteModelEntity to this model entity group.
  void AddModelEntity(vtkIdType EntityId);

  void AddModelEntity(vtkDiscreteModelEntity* Entity);

  // Description:
  // Clear the list of model entities to be added to
  // the model entity group.
  void ClearEntitiesToAdd();

  // Description:
  // Get the list of EntitiesToAdd.
  vtkGetObjectMacro(EntitiesToAdd, vtkIdList);

  // Description:
  // Set/get the model entity type that the built group will contain.
  // This is used by Build(), and can only be vtkModelFaceType or vtkModelEdgeType.
  vtkGetMacro(BuildEnityType, int);
  void SetBuildEnityType(int enType);

  // Description:
  // Remove a vtkDiscreteModelEntity from this model entity group.
  void RemoveModelEntity(vtkIdType EntityId);

  void RemoveModelEntity(vtkDiscreteModelEntity* Entity);

  // Description:
  // Clear the list of model entities to be removed from
  // the model entity group.
  void ClearEntitiesToRemove();

  // Description:
  // Get the list of EntitiesToRemove.
  vtkGetObjectMacro(EntitiesToRemove, vtkIdList);

  // Description:
  // Do the operations on the specified model entity group.
  bool Operate(vtkDiscreteModel* Model) override;

  // Description:
  // Build the model entity group on the server and client before operating
  // on it.  The function returns the UniquePersistenId of the
  // built model entity group.
  virtual vtkIdType Build(vtkDiscreteModel* Model);

  // Description:
  // Destroy the model entity group on the server and client if possible.
  virtual bool Destroy(vtkDiscreteModel* Model);

protected:
  vtkModelEntityGroupOperationBase();
  ~vtkModelEntityGroupOperationBase() override;

  // Description:
  // Check to see if everything is properly set for the operator.
  bool AbleToOperate(vtkDiscreteModel* Model) override;

private:
  vtkModelEntityGroupOperationBase(const vtkModelEntityGroupOperationBase&); // Not implemented.
  void operator=(const vtkModelEntityGroupOperationBase&);                   // Not implemented.

  // Description:
  // A list of vtkCMBModelEntitys to add.
  vtkIdList* EntitiesToAdd;

  // Description:
  // A list of vtkCMBModelEntitys to add.
  vtkIdList* EntitiesToRemove;

  // Description:
  // The type of entities group that will be built. Used by Build()
  int BuildEnityType;
};

#endif
