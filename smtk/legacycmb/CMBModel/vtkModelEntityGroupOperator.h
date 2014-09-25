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
// .NAME vtkModelEntityGroupOperator - Modify a model entity group on the server.
// .SECTION Description
// Operator to change information about a model entity group on the server.
// As this derives
// from vtkModelEntityOperator, you can use this to also modify RGBA,
// user name and/or visibility of a model entity group in addition to
// adding and/or removing model entities that are associated with
// a model entity group.

#ifndef __vtkModelEntityGroupOperator_h
#define __vtkModelEntityGroupOperator_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkModelEntityGroupOperatorBase.h"
#include "cmbSystemConfig.h"

#include <iostream>

class vtkDiscreteModelWrapper;
class vtkIdList;

class VTKCMBDISCRETEMODEL_EXPORT vtkModelEntityGroupOperator : public vtkModelEntityGroupOperatorBase
{
public:
  static vtkModelEntityGroupOperator * New();
  vtkTypeMacro(vtkModelEntityGroupOperator,vtkModelEntityGroupOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Prevent warnings about hidden base-class virtuals:
  using Superclass::Build;
  using Superclass::Destroy;
  using Superclass::Operate;

  // Description:
  // Modify the color, user name, and/or the visibility of an object.
  virtual void Operate(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Build a new model entity group on the server before operating
  // on it.
  virtual void Build(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Destroy the model entity group on the server if possible.
  virtual void Destroy(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Return the model entity.
  //vtkModelEntity* GetModelEntity(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

  // Description:
  // Returns the UniquePersistentId of the built vtkDiscreteModelEntityGroup.
  // Returns -1 if no entity group was built.
  vtkGetMacro(BuiltModelEntityGroupId, int);

  // Description:
  // Returns success (1) or failue (0) for Destroy.
  vtkGetMacro(DestroySucceeded, int);

protected:
  vtkModelEntityGroupOperator();
  virtual ~vtkModelEntityGroupOperator();

  using Superclass::AbleToOperate;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

private:
  vtkModelEntityGroupOperator(const vtkModelEntityGroupOperator&);  // Not implemented.
  void operator=(const vtkModelEntityGroupOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  // Description:
  // If a new vtkDiscreteModelEntityGroup was built with the Build(),
  // the built entity goup Id will be stored in BuiltModelEntityGroupId.
  vtkIdType BuiltModelEntityGroupId;

  // Description:
  // Flag to indicate that the destroy on the model succeeded (1) or not (0).
  int DestroySucceeded;
};

#endif
