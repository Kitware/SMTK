//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelEntityOperator - Change properties of model entities.
// .SECTION Description
// Operator to change the color (RGBA), user name, and/or visibility of a
// vtkModelEntity on the server.

#ifndef __vtkModelEntityOperator_h
#define __vtkModelEntityOperator_h

#include "cmbSystemConfig.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkModelEntityOperatorBase.h"

class vtkDiscreteModelWrapper;
class vtkModelEntity;

class VTKCMBDISCRETEMODEL_EXPORT vtkModelEntityOperator : public vtkModelEntityOperatorBase
{
public:
  static vtkModelEntityOperator * New();
  vtkTypeMacro(vtkModelEntityOperator,vtkModelEntityOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Prevent warnings about hidden base-class virtuals:
  using Superclass::Operate;
  using Superclass::GetModelEntity;

  // Description:
  // Modify the color, user name, and/or the visibility of an object.
  virtual void Operate(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Return the model entity.
  vtkModelEntity* GetModelEntity(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkModelEntityOperator();
  virtual ~vtkModelEntityOperator();

  using Superclass::AbleToOperate;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

private:
  vtkModelEntityOperator(const vtkModelEntityOperator&);  // Not implemented.
  void operator=(const vtkModelEntityOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
