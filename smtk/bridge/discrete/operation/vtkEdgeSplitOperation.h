//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkEdgeSplitOperation - Split a model edge on the server
// .SECTION Description
// Operation to split a model edge given a point id on the server.

#ifndef __smtkdiscrete_vtkEdgeSplitOperation_h
#define __smtkdiscrete_vtkEdgeSplitOperation_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkEdgeSplitOperationBase.h"

class vtkDiscreteModelWrapper;
class vtkIdTypeArray;
class vtkModelEntity;

class SMTKDISCRETESESSION_EXPORT vtkEdgeSplitOperation : public vtkEdgeSplitOperationBase
{
public:
  static vtkEdgeSplitOperation* New();
  vtkTypeMacro(vtkEdgeSplitOperation, vtkEdgeSplitOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Modify the color and/or the visibility of an object.
  virtual void Operate(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Return the model entity.
  vtkModelEntity* GetModelEntity(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkEdgeSplitOperation();
  ~vtkEdgeSplitOperation() override;

  using Superclass::AbleToOperate;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

private:
  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  vtkEdgeSplitOperation(const vtkEdgeSplitOperation&); // Not implemented.
  void operator=(const vtkEdgeSplitOperation&);        // Not implemented.
};

#endif
