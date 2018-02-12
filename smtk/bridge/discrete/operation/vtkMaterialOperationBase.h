//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkMaterialOperationBase - Modify a material.
// .SECTION Description
// Operation to change information about a material.  As this derives
// from vtkModelEntityOperation, you can use this to also modify RGBA,
// user name and/or visibility of a material in addition to
// adding and/or removing model entities that are associated with
// a material.
// We may eventually need to add in the ability to set a
// warehouseid of a material here as well.

#ifndef __smtkdiscrete_vtkMaterialOperationBase_h
#define __smtkdiscrete_vtkMaterialOperationBase_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkModelEntityOperationBase.h"

class vtkModelMaterial;
class vtkDiscreteModel;
class vtkIdList;
class vtkModelEntity;
class vtkModelGeometricEntity;

class SMTKDISCRETESESSION_EXPORT vtkMaterialOperationBase : public vtkModelEntityOperationBase
{
public:
  static vtkMaterialOperationBase* New();
  vtkTypeMacro(vtkMaterialOperationBase, vtkModelEntityOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set the model entity type to vtkModelMaterialType to override
  // the superclass virtual function.
  void SetItemType(int itemType) override;

  // Description:
  // Get the material from Model.
  vtkModelMaterial* GetMaterial(vtkDiscreteModel* Model);

  // Description:
  // Add a vtkCMBGeometricModelEntity to this material.  Note that
  // only model regions and model faces that are not adjacent to
  // model regions should be added to a material.  Note that the
  // only way to remove a vtkCMBGeometricModelEntity from a material
  // is by adding it to another material.
  void AddModelGeometricEntity(vtkIdType GeometricEntityId);

  void AddModelGeometricEntity(vtkModelGeometricEntity* GeometricEntity);

  // Description:
  // Clear the list of model geometric entities to be added to
  // the material.
  void ClearGeometricEntitiesToAdd();

  // Description:
  // Get the list of GeometricEntitiesToAdd.
  vtkGetObjectMacro(GeometricEntitiesToAdd, vtkIdList);

  // Description:
  // Remove Entities. This is not present in V3
  void ClearGeometricEntitiesToRemove();
  void RemoveModelGeometricEntity(vtkIdType GeometricEntityId);
  void RemoveModelGeometricEntity(vtkModelGeometricEntity* GeometricEntity);
  vtkGetObjectMacro(GeometricEntitiesToRemove, vtkIdList);

  // Description:
  // Get the list of PreviousMaterialsOfGeometricEntities.
  vtkGetObjectMacro(PreviousMaterialsOfGeometricEntities, vtkIdList);

  // Description:
  // Do the operations on the specified material.
  bool Operate(vtkDiscreteModel* Model) override;

  // Description:
  // Build the material on the server and client before operating
  // on it.  The function returns the UniquePersistenId of the
  // built material.
  virtual vtkIdType Build(vtkDiscreteModel* Model);

  // Description:
  // Destroy the material on the server and client if possible.
  virtual bool Destroy(vtkDiscreteModel* Model);

protected:
  vtkMaterialOperationBase();
  ~vtkMaterialOperationBase() override;

  // Description:
  // Check to see if everything is properly set for the operator.
  bool AbleToOperate(vtkDiscreteModel* Model) override;

private:
  vtkMaterialOperationBase(const vtkMaterialOperationBase&); // Not implemented.
  void operator=(const vtkMaterialOperationBase&);           // Not implemented.

  // Description:
  // A list of vtkModelGeometricEntitys to add/remove.
  vtkIdList* GeometricEntitiesToAdd;
  vtkIdList* GeometricEntitiesToRemove;

  // Description:
  // A list of the old materials that each vtkModelGeometricEntity
  // was associated with.
  vtkIdList* PreviousMaterialsOfGeometricEntities;
};

#endif
