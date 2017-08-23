//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkSelectionSplitOperator - Split a model face based on a vtkSelection.
// .SECTION Description
// Operator to split model faces based on a vtkSelection. Note
// that the cell Ids included in the vtkSelection are with respect
// to the master vtkPolyData.

#ifndef __smtkdiscrete_vtkSelectionSplitOperator_h
#define __smtkdiscrete_vtkSelectionSplitOperator_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkSelectionSplitOperatorBase.h"

class vtkDiscreteModel;
class vtkDiscreteModelWrapper;
class vtkIdTypeArray;
class vtkSelectionAlgorithm;
class vtkSelection;

class SMTKDISCRETESESSION_EXPORT vtkSelectionSplitOperator : public vtkSelectionSplitOperatorBase
{
public:
  static vtkSelectionSplitOperator* New();
  vtkTypeMacro(vtkSelectionSplitOperator, vtkSelectionSplitOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Split the model faces based on the input Selection.
  virtual void Operate(vtkDiscreteModelWrapper* ModelWrapper, vtkSelectionAlgorithm* Selection);
  virtual void Operate(vtkDiscreteModelWrapper* modelWrapper, vtkSelection* selection);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkSelectionSplitOperator();
  ~vtkSelectionSplitOperator() override;

  using Superclass::AbleToOperate;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

private:
  // Description:
  // Return whether or not the operation successfully completed.
  int OperateSucceeded;

  vtkSelectionSplitOperator(const vtkSelectionSplitOperator&); // Not implemented.
  void operator=(const vtkSelectionSplitOperator&);            // Not implemented.
};

#endif
