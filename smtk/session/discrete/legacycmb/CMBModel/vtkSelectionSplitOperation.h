//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkSelectionSplitOperation - Split a model face based on a vtkSelection.
// .SECTION Description
// Operation to split model faces based on a vtkSelection. Note
// that the cell Ids included in the vtkSelection are with respect
// to the master vtkPolyData.

#ifndef __vtkSelectionSplitOperation_h
#define __vtkSelectionSplitOperation_h

#include "cmbSystemConfig.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkSelectionSplitOperationBase.h"

class vtkDiscreteModel;
class vtkDiscreteModelWrapper;
class vtkIdTypeArray;
class vtkSelectionAlgorithm;

class VTKCMBDISCRETEMODEL_EXPORT vtkSelectionSplitOperation : public vtkSelectionSplitOperationBase
{
public:
  static vtkSelectionSplitOperation* New();
  vtkTypeMacro(vtkSelectionSplitOperation, vtkSelectionSplitOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Split the model faces based on the input Selection.
  virtual void Operate(vtkDiscreteModelWrapper* ModelWrapper, vtkSelectionAlgorithm* Selection);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkSelectionSplitOperation();
  virtual ~vtkSelectionSplitOperation();

  using Superclass::AbleToOperate;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

private:
  // Description:
  // Return whether or not the operation successfully completed.
  int OperateSucceeded;

  vtkSelectionSplitOperation(const vtkSelectionSplitOperation&); // Not implemented.
  void operator=(const vtkSelectionSplitOperation&);             // Not implemented.
};

#endif
