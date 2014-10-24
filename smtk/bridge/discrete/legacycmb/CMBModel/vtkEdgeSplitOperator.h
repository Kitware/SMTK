//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

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
