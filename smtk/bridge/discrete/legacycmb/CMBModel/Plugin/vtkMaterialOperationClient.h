//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkMaterialOperationClient - Modify a material.
// .SECTION Description
// Operation to build, destroy, and/or change information about a material.
// As this derives
// from vtkModelEntityOperation, you can use this to also modify RGBA,
// user name and/or visibility of a material in addition to
// adding and/or removing model entities that are associated with
// a material.  If Build is used to create a new vtkCMBMaterial,
// the Id of the newly created material will be set as the
// Id to operate on.  Note that a material can only be destroyed
// if it has no model entities associated with it (otherwise,
// those model entities would have an undefined material making
// the vtkDiscreteModel invalid).
// We may eventually need to add in the ability to set a
// warehouseid of a material here as well.

#ifndef __vtkMaterialOperationClient_h
#define __vtkMaterialOperationClient_h

#include "cmbSystemConfig.h"
#include "vtkMaterialOperationBase.h"

class vtkCMBMaterial;
class vtkDiscreteModel;
class vtkIdList;
class vtkModelEntity;
class vtkModelGeometricEntity;
class vtkSMProxy;

class VTK_EXPORT vtkMaterialOperationClient : public vtkMaterialOperationBase
{
public:
  static vtkMaterialOperationClient* New();
  vtkTypeMacro(vtkMaterialOperationClient, vtkMaterialOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Prevent warnings about hidden base-class virtuals:
  using Superclass::Build;
  using Superclass::Destroy;
  using Superclass::Operate;

  // Description:
  // Do the operations on the specified material.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

  // Description:
  // Build the material on the server and client before operating
  // on it.  The function returns the UniquePersistenId of the
  // built material.
  virtual vtkIdType Build(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

  // Description:
  // Destroy the material on the server and client if possible.
  virtual bool Destroy(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

protected:
  vtkMaterialOperationClient();
  virtual ~vtkMaterialOperationClient();

private:
  vtkMaterialOperationClient(const vtkMaterialOperationClient&); // Not implemented.
  void operator=(const vtkMaterialOperationClient&);             // Not implemented.
};

#endif
