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
// .NAME vtkSelectionSplitOperator - Split a model face based on a vtkSelection.
// .SECTION Description
// Operator to split model faces based on a vtkSelection. Note
// that the cell Ids included in the vtkSelection are with respect
// to the master vtkPolyData.

#ifndef __vtkSelectionSplitOperator_h
#define __vtkSelectionSplitOperator_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkSelectionSplitOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkDiscreteModelWrapper;
class vtkIdTypeArray;
class vtkSelectionAlgorithm;

class VTKCMBDISCRETEMODEL_EXPORT vtkSelectionSplitOperator : public vtkSelectionSplitOperatorBase
{
public:
  static vtkSelectionSplitOperator * New();
  vtkTypeMacro(vtkSelectionSplitOperator,vtkSelectionSplitOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Split the model faces based on the input Selection.
  virtual void Operate(vtkDiscreteModelWrapper* ModelWrapper,
                       vtkSelectionAlgorithm* Selection);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkSelectionSplitOperator();
  virtual ~vtkSelectionSplitOperator();

  using Superclass::AbleToOperate;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

private:
  // Description:
  // Return whether or not the operation successfully completed.
  int OperateSucceeded;

  vtkSelectionSplitOperator(const vtkSelectionSplitOperator&);  // Not implemented.
  void operator=(const vtkSelectionSplitOperator&);  // Not implemented.
};

#endif
