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
// .NAME vtkModelEntityOperator - Change properties of model entities.
// .SECTION Description
// Operator to change the color (RGBA), user name, and/or visibility of a
// vtkModelEntity on the server.

#ifndef __vtkModelEntityOperator_h
#define __vtkModelEntityOperator_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkModelEntityOperatorBase.h"
#include "cmbSystemConfig.h"

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

//BTX
  // Description:
  // Return the model entity.
  vtkModelEntity* GetModelEntity(vtkDiscreteModelWrapper* ModelWrapper);
//ETX

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
