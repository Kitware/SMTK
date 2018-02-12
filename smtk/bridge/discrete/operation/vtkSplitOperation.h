//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkSplitOperation - Split a model face on the server
// .SECTION Description
// Operation to split a model face given an angle on the server.

#ifndef __smtkdiscrete_vtkSplitOperation_h
#define __smtkdiscrete_vtkSplitOperation_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkSplitOperationBase.h"

class vtkDiscreteModelWrapper;
class vtkIdTypeArray;
class vtkModelEntity;

class SMTKDISCRETESESSION_EXPORT vtkSplitOperation : public vtkSplitOperationBase
{
public:
  static vtkSplitOperation* New();
  vtkTypeMacro(vtkSplitOperation, vtkSplitOperationBase);
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
  vtkSplitOperation();
  ~vtkSplitOperation() override;

  using Superclass::AbleToOperate;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

private:
  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  vtkSplitOperation(const vtkSplitOperation&); // Not implemented.
  void operator=(const vtkSplitOperation&);    // Not implemented.
};

#endif
