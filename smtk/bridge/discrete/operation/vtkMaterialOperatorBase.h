//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkMaterialOperatorBase - Modify a material.
// .SECTION Description
// Operator to change information about a material.  As this derives
// from vtkModelEntityOperator, you can use this to also modify RGBA,
// user name and/or visibility of a material in addition to
// adding and/or removing model entities that are associated with
// a material.
// We may eventually need to add in the ability to set a
// warehouseid of a material here as well.

#ifndef __smtkdiscrete_vtkMaterialOperatorBase_h
#define __smtkdiscrete_vtkMaterialOperatorBase_h

#include "smtk/bridge/discrete/discreteSessionExports.h" // For export macro
#include "vtkModelEntityOperatorBase.h"

class vtkModelMaterial;
class vtkDiscreteModel;
class vtkIdList;
class vtkModelEntity;
class vtkModelGeometricEntity;

class SMTKDISCRETESESSION_EXPORT vtkMaterialOperatorBase : public vtkModelEntityOperatorBase
{
public:
  static vtkMaterialOperatorBase * New();
  vtkTypeMacro(vtkMaterialOperatorBase,vtkModelEntityOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the model entity type to vtkModelMaterialType to override
  // the superclass virtual function.
  virtual void SetItemType(int itemType);

//BTX
  // Description:
  // Get the material from Model.
  vtkModelMaterial* GetMaterial(vtkDiscreteModel* Model);
//ETX

  // Description:
  // Add a vtkCMBGeometricModelEntity to this material.  Note that
  // only model regions and model faces that are not adjacent to
  // model regions should be added to a material.  Note that the
  // only way to remove a vtkCMBGeometricModelEntity from a material
  // is by adding it to another material.
  void AddModelGeometricEntity(vtkIdType GeometricEntityId);
//BTX
  void AddModelGeometricEntity(vtkModelGeometricEntity* GeometricEntity);
//ETX

  // Description:
  // Clear the list of model geometric entities to be added to
  // the material.
  void ClearGeometricEntitiesToAdd();

  // Description:
  // Get the list of GeometricEntitiesToAdd.
  vtkGetObjectMacro(GeometricEntitiesToAdd, vtkIdList);

  // Description:
  // Get the list of PreviousMaterialsOfGeometricEntities.
  vtkGetObjectMacro(PreviousMaterialsOfGeometricEntities, vtkIdList);

//BTX
  // Description:
  // Do the operations on the specified material.
  virtual bool Operate(vtkDiscreteModel* Model);

  // Description:
  // Build the material on the server and client before operating
  // on it.  The function returns the UniquePersistenId of the
  // built material.
  virtual vtkIdType Build(vtkDiscreteModel* Model);

  // Description:
  // Destroy the material on the server and client if possible.
  virtual bool Destroy(vtkDiscreteModel* Model);
//ETX

protected:
  vtkMaterialOperatorBase();
  virtual ~vtkMaterialOperatorBase();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

private:
  vtkMaterialOperatorBase(const vtkMaterialOperatorBase&);  // Not implemented.
  void operator=(const vtkMaterialOperatorBase&);  // Not implemented.

  // Description:
  // A list of vtkModelGeometricEntitys to add.
  vtkIdList* GeometricEntitiesToAdd;

  // Description:
  // A list of the old materials that each vtkModelGeometricEntity
  // was associated with.
  vtkIdList* PreviousMaterialsOfGeometricEntities;
};

#endif
