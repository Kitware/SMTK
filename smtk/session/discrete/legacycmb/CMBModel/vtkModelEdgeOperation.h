//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelEdgeOperation - Change properties of model entities.
// .SECTION Description
// Operation to change line resolution, the color (RGBA), user name,
// and/or visibility of a vtkModelEdge on the server.

#ifndef __vtkModelEdgeOperation_h
#define __vtkModelEdgeOperation_h

#include "cmbSystemConfig.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkModelEdgeOperationBase.h"

class vtkDiscreteModelWrapper;

class VTKCMBDISCRETEMODEL_EXPORT vtkModelEdgeOperation : public vtkModelEdgeOperationBase
{
public:
  static vtkModelEdgeOperation* New();
  vtkTypeMacro(vtkModelEdgeOperation, vtkModelEdgeOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  using Superclass::Operate;

  // Description:
  // Modify the color, user name, and/or the visibility of an object.
  virtual void Operate(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkModelEdgeOperation();
  virtual ~vtkModelEdgeOperation();

  using Superclass::AbleToOperate;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

private:
  vtkModelEdgeOperation(const vtkModelEdgeOperation&); // Not implemented.
  void operator=(const vtkModelEdgeOperation&);        // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
