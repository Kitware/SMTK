/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME vtkMaterialOperatorBase - Modify a material.
// .SECTION Description
// Operator to change information about a material.  As this derives
// from vtkModelEntityOperator, you can use this to also modify RGBA,
// user name and/or visibility of a material in addition to
// adding and/or removing model entities that are associated with
// a material.
// We may eventually need to add in the ability to set a
// warehouseid of a material here as well.


#ifndef __vtkMaterialOperatorBase_h
#define __vtkMaterialOperatorBase_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkModelEntityOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkModelMaterial;
class vtkDiscreteModel;
class vtkIdList;
class vtkModelEntity;
class vtkModelGeometricEntity;

class VTKCMBDISCRETEMODEL_EXPORT vtkMaterialOperatorBase : public vtkModelEntityOperatorBase
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
