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
// .NAME vtkMergeOperator - Merge a set of geometric model entities
// .SECTION Description
// Operator to merge a set of source geometric model entities into
// a target geometric entity on the server.  The properties of the target entity
// (e.g. color, BCS/ModelEntityGroup associations) will not be changed.
// Warning: This may only currently work from model faces.

#ifndef __vtkMergeOperator_h
#define __vtkMergeOperator_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkMergeOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;

class VTKCMBDISCRETEMODEL_EXPORT vtkMergeOperator : public vtkMergeOperatorBase
{
public:
  static vtkMergeOperator * New();
  vtkTypeMacro(vtkMergeOperator,vtkMergeOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  using Superclass::Operate;

  // Description:
  // Modify the color, user name, and/or the visibility of an object.
  virtual void Operate(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

//BTX
  // Description:
  // Get the target geometric model entity.
  vtkDiscreteModelGeometricEntity* GetTargetModelEntity(vtkDiscreteModelWrapper*);
//ETX

protected:
  vtkMergeOperator();
  virtual ~vtkMergeOperator();

  virtual bool AbleToOperate(vtkDiscreteModel* model)
    { return this->Superclass::AbleToOperate(model); }

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

private:
  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  vtkMergeOperator(const vtkMergeOperator&);  // Not implemented.
  void operator=(const vtkMergeOperator&);  // Not implemented.
};

#endif
