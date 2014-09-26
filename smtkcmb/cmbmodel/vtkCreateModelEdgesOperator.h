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
// .NAME vtkCreateModelEdgesOperator -Create Model Edges on the server
// .SECTION Description
// Operator to create model edges on the server for a model that contains model regions

#ifndef __smtkcmb_vtkCreateModelEdgesOperator_h
#define __smtkcmb_vtkCreateModelEdgesOperator_h

#include "vtkSMTKCMBModelModule" // For export macro
#include "vtkCreateModelEdgesOperatorBase.h"
#include "ModelEdgeHelper.h" // for NewModelEdgeInfo and LoopInfo


class vtkIdTypeArray;
class vtkDiscreteModelWrapper;
class vtkDiscreteModel;
class vtkDiscreteModelEdge;
class vtkPolyData;

class VTKSMTKCMBMODEL_EXPORT vtkCreateModelEdgesOperator :
  public vtkCreateModelEdgesOperatorBase
{
public:
  class NewModelEdgeInfo;
  class LoopInfo;
  static vtkCreateModelEdgesOperator * New();
  vtkTypeMacro(vtkCreateModelEdgesOperator,vtkCreateModelEdgesOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create the model edges
  virtual void Operate(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

  // Description:
  // Set the ShowEdges flag, default to off.
  vtkSetMacro(ShowEdges, int);
  vtkGetMacro(ShowEdges, int);
  vtkBooleanMacro(ShowEdges,int);

protected:
  vtkCreateModelEdgesOperator();
  virtual ~vtkCreateModelEdgesOperator();

  using Superclass::AbleToOperate;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

private:
  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
  vtkDiscreteModel *Model;
  int ShowEdges;

  vtkCreateModelEdgesOperator(const vtkCreateModelEdgesOperator&);  // Not implemented.
  void operator=(const vtkCreateModelEdgesOperator&);  // Not implemented.
};

#endif
