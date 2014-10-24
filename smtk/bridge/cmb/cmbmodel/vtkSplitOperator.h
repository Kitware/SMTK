//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkSplitOperator - Split a model face on the server
// .SECTION Description
// Operator to split a model face given an angle on the server.

#ifndef __smtkdiscrete_vtkSplitOperator_h
#define __smtkdiscrete_vtkSplitOperator_h

#include "smtk/bridge/cmb/discreteBridgeExports.h" // For export macro
#include "vtkSplitOperatorBase.h"


class vtkDiscreteModelWrapper;
class vtkIdTypeArray;
class vtkModelEntity;

class SMTKDISCRETEBRIDGE_EXPORT vtkSplitOperator : public vtkSplitOperatorBase
{
public:
  static vtkSplitOperator * New();
  vtkTypeMacro(vtkSplitOperator,vtkSplitOperatorBase);
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
  vtkSplitOperator();
  virtual ~vtkSplitOperator();

  using Superclass::AbleToOperate;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

private:
  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  vtkSplitOperator(const vtkSplitOperator&);  // Not implemented.
  void operator=(const vtkSplitOperator&);  // Not implemented.
};

#endif
