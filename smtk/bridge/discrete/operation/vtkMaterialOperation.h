//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkMaterialOperation - Modify a material on the server.
// .SECTION Description
// Operation to change information about a material on the server.
// As this derives
// from vtkModelEntityOperation, you can use this to also modify RGBA,
// user name and/or visibility of a material in addition to
// adding and/or removing model entities that are associated with
// a material. We may eventually need to add in the ability to set a
// warehouseid of a material here as well.

#ifndef __vtkMaterialOperation_h
#define __vtkMaterialOperation_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkMaterialOperationBase.h"

#include <iostream>

class vtkDiscreteModel;
class vtkDiscreteModelGeometricEntity;
class vtkDiscreteModelWrapper;
class vtkIdList;
class vtkModelEntity;

class SMTKDISCRETESESSION_EXPORT vtkMaterialOperation : public vtkMaterialOperationBase
{
public:
  static vtkMaterialOperation* New();
  vtkTypeMacro(vtkMaterialOperation, vtkMaterialOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Prevent warnings about hidden base-class virtuals:
  using Superclass::AbleToOperate;
  using Superclass::Build;
  using Superclass::Destroy;
  using Superclass::Operate;

  // Description:
  // Modify the color, user name, and/or the visibility of an object.
  virtual void Operate(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Build the material on the server before operating
  // on it.
  virtual void Build(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Destroy the material on the server if possible.
  virtual void Destroy(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Return the model entity.
  //vtkModelEntity* GetModelEntity(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

  // Description:
  // Returns the UniquePersistentId of the created Material.  Returns
  // -1 if no material was built.
  vtkGetMacro(BuiltMaterialId, int);

  // Description:
  // Returns success (1) or failue (0) for Destroy.
  vtkGetMacro(DestroySucceeded, int);

protected:
  vtkMaterialOperation();
  ~vtkMaterialOperation() override;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

private:
  vtkMaterialOperation(const vtkMaterialOperation&); // Not implemented.
  void operator=(const vtkMaterialOperation&);       // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  // Description:
  // If a new material was built with Build(), the built material Id
  // will be stored in BuiltMaterialId.
  vtkIdType BuiltMaterialId;

  // Description:
  // Flag to indicate that the destroy on the model succeeded (1) or not (0).
  int DestroySucceeded;
};

#endif
