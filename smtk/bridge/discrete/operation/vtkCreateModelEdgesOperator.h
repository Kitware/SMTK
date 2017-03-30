//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCreateModelEdgesOperator -Create Model Edges on the server
// .SECTION Description
// Operator to create model edges on the server for a model that contains model regions

#ifndef __smtkdiscrete_vtkCreateModelEdgesOperator_h
#define __smtkdiscrete_vtkCreateModelEdgesOperator_h

#include "ModelEdgeHelper.h"              // for NewModelEdgeInfo and LoopInfo
#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkCreateModelEdgesOperatorBase.h"

class vtkIdTypeArray;
class vtkDiscreteModelWrapper;
class vtkDiscreteModel;
class vtkDiscreteModelEdge;
class vtkPolyData;

class SMTKDISCRETESESSION_EXPORT vtkCreateModelEdgesOperator
  : public vtkCreateModelEdgesOperatorBase
{
public:
  class NewModelEdgeInfo;
  class LoopInfo;
  static vtkCreateModelEdgesOperator* New();
  vtkTypeMacro(vtkCreateModelEdgesOperator, vtkCreateModelEdgesOperatorBase);
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
  vtkBooleanMacro(ShowEdges, int);

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
  vtkDiscreteModel* Model;
  int ShowEdges;

  vtkCreateModelEdgesOperator(const vtkCreateModelEdgesOperator&); // Not implemented.
  void operator=(const vtkCreateModelEdgesOperator&);              // Not implemented.
};

#endif
