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
// .NAME vtkEnclosingModelEntityOperator -
// .SECTION Description


#ifndef __vtkEnclosingModelEntityOperator_h
#define __vtkEnclosingModelEntityOperator_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;
//class vtkIdTypeArray;
class vtkModelEntity;
class vtkCellLocator;

class VTKCMBDISCRETEMODEL_EXPORT vtkEnclosingModelEntityOperator : public vtkObject
{
public:
  static vtkEnclosingModelEntityOperator * New();
  vtkTypeMacro(vtkEnclosingModelEntityOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  void BuildLinks(vtkDiscreteModelWrapper* modelWrapper);

  // Description:
  // Modify the color and/or the visibility of an object.
  virtual void Operate(vtkDiscreteModelWrapper* modelWrapper, double *pt);

  // Description:
  // Return the model entity.
  vtkGetObjectMacro(EnclosingEntity, vtkModelEntity);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkEnclosingModelEntityOperator();
  virtual ~vtkEnclosingModelEntityOperator();

private:
  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  vtkCellLocator *CellLocator;
  vtkModelEntity *EnclosingEntity;

  vtkEnclosingModelEntityOperator(const vtkEnclosingModelEntityOperator&);  // Not implemented.
  void operator=(const vtkEnclosingModelEntityOperator&);  // Not implemented.
};

#endif
