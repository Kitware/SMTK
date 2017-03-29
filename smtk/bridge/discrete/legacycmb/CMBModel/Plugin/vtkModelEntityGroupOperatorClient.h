//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelEntityGroupOperatorClient - Modify a model entity group.
// .SECTION Description
// Operator to build, destroy, and/or change information about a
// vtkCMBModelEntityGroup.  As this derives
// from vtkModelEntityOperator, you can use this to also modify RGBA,
// user name and/or visibility of an entity group in addition to
// adding and/or removing model entities that are associated with
// an entity group.  If Build is used to create a new
// vtkCMBModelEntityGroup, the Id of the newly created entity group
// will be set as the Id to operate on.

#ifndef __vtkModelEntityGroupOperatorClient_h
#define __vtkModelEntityGroupOperatorClient_h

#include "cmbSystemConfig.h"
#include "vtkModelEntityGroupOperatorBase.h"

class vtkDiscreteModel;
class vtkCMBModelEntity;
class vtkCMBModelEntityGroup;
class vtkIdList;
class vtkModelEntity;
class vtkSMProxy;

class VTK_EXPORT vtkModelEntityGroupOperatorClient :
  public vtkModelEntityGroupOperatorBase
{
public:
  static vtkModelEntityGroupOperatorClient * New();
  vtkTypeMacro(vtkModelEntityGroupOperatorClient,vtkModelEntityGroupOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Prevent warnings about hidden base-class virtuals:
  using Superclass::Build;
  using Superclass::Destroy;
  using Superclass::Operate;

  // Description:
  // Do the operations on the specified entity group.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

  // Description:
  // Build the entity group on the server and client before operating
  // on it.  The function returns the UniquePersistenId of the
  // built model entity group.
  virtual vtkIdType Build(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

  // Description:
  // Destroy the entity group on the server and client if possible.
  virtual bool Destroy(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

protected:
  vtkModelEntityGroupOperatorClient();
  virtual ~vtkModelEntityGroupOperatorClient();

private:
  vtkModelEntityGroupOperatorClient(const vtkModelEntityGroupOperatorClient&);  // Not implemented.
  void operator=(const vtkModelEntityGroupOperatorClient&);  // Not implemented.
};

#endif
