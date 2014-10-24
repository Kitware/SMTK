//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelEdgeOperator - Change properties of model entities.
// .SECTION Description
// Operator to change line resolution, the color (RGBA), user name,
// and/or visibility of a vtkModelEdge on the server.

#ifndef __vtkModelEdgeOperator_h
#define __vtkModelEdgeOperator_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkModelEdgeOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;

class VTKCMBDISCRETEMODEL_EXPORT vtkModelEdgeOperator : public vtkModelEdgeOperatorBase
{
public:
  static vtkModelEdgeOperator * New();
  vtkTypeMacro(vtkModelEdgeOperator,vtkModelEdgeOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  using Superclass::Operate;

  // Description:
  // Modify the color, user name, and/or the visibility of an object.
  virtual void Operate(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkModelEdgeOperator();
  virtual ~vtkModelEdgeOperator();

  using Superclass::AbleToOperate;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

private:
  vtkModelEdgeOperator(const vtkModelEdgeOperator&);  // Not implemented.
  void operator=(const vtkModelEdgeOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

};

#endif
