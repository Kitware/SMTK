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
// .NAME vtkCMBModelPointsOperator -
// .SECTION Description
// An operator to only SetPoints for model geometry, and the cell structure
// is unchanged.

#ifndef __vtkCMBModelPointsOperator_h
#define __vtkCMBModelPointsOperator_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;
class vtkPointSet;
class vtkPointData;
class vtkAlgorithm;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBModelPointsOperator : public vtkObject
{
public:
  static vtkCMBModelPointsOperator * New();
  vtkTypeMacro(vtkCMBModelPointsOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Load the file into Model.
  void Operate(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Set model points
  void SetModelPointsInput(vtkAlgorithm* dataAlg);
  void SetModelPoints(vtkPointSet* dataObj);
  vtkGetObjectMacro(ModelPoints, vtkPointSet);

  // Description:
  // Set model points
  void SetModelPointDataInput(vtkAlgorithm* dataAlg);
  void SetModelPointData(vtkPointData* pointData);
  vtkGetObjectMacro(ModelPointData, vtkPointData);

  // Description:
  // Returns success (1) or failure (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkCMBModelPointsOperator();
  virtual ~vtkCMBModelPointsOperator();

private:

  vtkCMBModelPointsOperator(const vtkCMBModelPointsOperator&);  // Not implemented.
  void operator=(const vtkCMBModelPointsOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
  vtkPointSet* ModelPoints;
  vtkPointData* ModelPointData;

};

#endif
