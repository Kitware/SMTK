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
// .NAME vtkEdgeSplitOperator - Split a model edge on the server
// .SECTION Description
// Operator to split a model edge given a point id on the server.

#ifndef __vtkEdgeSplitOperator_h
#define __vtkEdgeSplitOperator_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkEdgeSplitOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;
class vtkIdTypeArray;
class vtkModelEntity;

class VTKCMBDISCRETEMODEL_EXPORT vtkEdgeSplitOperator : public vtkEdgeSplitOperatorBase
{
public:
  static vtkEdgeSplitOperator * New();
  vtkTypeMacro(vtkEdgeSplitOperator,vtkEdgeSplitOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Modify the color and/or the visibility of an object.
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
  vtkEdgeSplitOperator();
  virtual ~vtkEdgeSplitOperator();

  using Superclass::AbleToOperate;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

private:
  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  vtkEdgeSplitOperator(const vtkEdgeSplitOperator&);  // Not implemented.
  void operator=(const vtkEdgeSplitOperator&);  // Not implemented.
};

#endif
