//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCreateModelEdgesOperation -Create Model Edges on the server
// .SECTION Description
// Operation to create model edges on the server for a model that contains model regions

#ifndef __smtkdiscrete_vtkCreateModelEdgesOperation_h
#define __smtkdiscrete_vtkCreateModelEdgesOperation_h

#include "smtk/session/discrete/Exports.h"                // For export macro
#include "smtk/session/discrete/kernel/ModelEdgeHelper.h" // for NewModelEdgeInfo and LoopInfo
#include "vtkCreateModelEdgesOperationBase.h"

class vtkIdTypeArray;
class vtkDiscreteModelWrapper;
class vtkDiscreteModel;
class vtkDiscreteModelEdge;
class vtkPolyData;

class SMTKDISCRETESESSION_EXPORT vtkCreateModelEdgesOperation
  : public vtkCreateModelEdgesOperationBase
{
public:
  class NewModelEdgeInfo;
  class LoopInfo;
  static vtkCreateModelEdgesOperation* New();
  vtkTypeMacro(vtkCreateModelEdgesOperation, vtkCreateModelEdgesOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

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
  vtkBooleanMacro(ShowEdges, int);

protected:
  vtkCreateModelEdgesOperation();
  ~vtkCreateModelEdgesOperation() override;

  using Superclass::AbleToOperate;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

private:
  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
  vtkDiscreteModel* Model;
  int ShowEdges;

  vtkCreateModelEdgesOperation(const vtkCreateModelEdgesOperation&); // Not implemented.
  void operator=(const vtkCreateModelEdgesOperation&);               // Not implemented.
};

#endif
