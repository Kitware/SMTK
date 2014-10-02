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
// .NAME vtkModelEntityGroupOperatorBase - Modify a model entity group.
// .SECTION Description
// Operator to change information about a model entity group.  As this
// derives from vtkModelEntityOperator, you can use
// this to also modify RGBA,
// user name and/or visibility of a model entity group in addition to
// adding and/or removing model entities that are associated with
// a model entity group.

#ifndef __vtkModelEntityGroupOperatorBase_h
#define __vtkModelEntityGroupOperatorBase_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkModelEntityOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelEntityGroup;
class vtkDiscreteModel;
class vtkDiscreteModelEntity;
class vtkIdList;
class vtkModelEntity;

class VTKCMBDISCRETEMODEL_EXPORT vtkModelEntityGroupOperatorBase : public vtkModelEntityOperatorBase
{
public:
  static vtkModelEntityGroupOperatorBase * New();
  vtkTypeMacro(vtkModelEntityGroupOperatorBase,vtkModelEntityOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the model entity type to vtkDiscreteModelEntityGroupType to override
  // the superclass virtual function.
  virtual void SetItemType(int itemType);

//BTX
  // Description:
  // Get the model entity group from Model.
  vtkDiscreteModelEntityGroup* GetModelEntityGroup(vtkDiscreteModel* Model);
//ETX

  // Description:
  // Add a vtkDiscreteModelEntity to this model entity group.
  void AddModelEntity(vtkIdType EntityId);
//BTX
  void AddModelEntity(vtkDiscreteModelEntity* Entity);
//ETX

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
//BTX
  void RemoveModelEntity(vtkDiscreteModelEntity* Entity);
//ETX

  // Description:
  // Clear the list of model entities to be removed from
  // the model entity group.
  void ClearEntitiesToRemove();

  // Description:
  // Get the list of EntitiesToRemove.
  vtkGetObjectMacro(EntitiesToRemove, vtkIdList);

//BTX
  // Description:
  // Do the operations on the specified model entity group.
  virtual bool Operate(vtkDiscreteModel* Model);

  // Description:
  // Build the model entity group on the server and client before operating
  // on it.  The function returns the UniquePersistenId of the
  // built model entity group.
  virtual vtkIdType Build(vtkDiscreteModel* Model);

  // Description:
  // Destroy the model entity group on the server and client if possible.
  virtual bool Destroy(vtkDiscreteModel* Model);
//ETX

protected:
  vtkModelEntityGroupOperatorBase();
  virtual ~vtkModelEntityGroupOperatorBase();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

private:
  vtkModelEntityGroupOperatorBase(const vtkModelEntityGroupOperatorBase&);  // Not implemented.
  void operator=(const vtkModelEntityGroupOperatorBase&);  // Not implemented.

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
