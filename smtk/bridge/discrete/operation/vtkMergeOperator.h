//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkMergeOperator - Merge a set of geometric model entities
// .SECTION Description
// Operator to merge a set of source geometric model entities into
// a target geometric entity on the server.  The properties of the target entity
// (e.g. color, BCS/ModelEntityGroup associations) will not be changed.
// Warning: This may only currently work from model faces.

#ifndef __smtkdiscrete_vtkMergeOperator_h
#define __smtkdiscrete_vtkMergeOperator_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkMergeOperatorBase.h"


class vtkDiscreteModelWrapper;

class SMTKDISCRETESESSION_EXPORT vtkMergeOperator : public vtkMergeOperatorBase
{
public:
  static vtkMergeOperator * New();
  vtkTypeMacro(vtkMergeOperator,vtkMergeOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  using Superclass::Operate;

  // Description:
  // Modify the color, user name, and/or the visibility of an object.
  virtual void Operate(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

//BTX
  // Description:
  // Get the target geometric model entity.
  vtkDiscreteModelGeometricEntity* GetTargetModelEntity(vtkDiscreteModelWrapper*);
//ETX

protected:
  vtkMergeOperator();
  virtual ~vtkMergeOperator();

  virtual bool AbleToOperate(vtkDiscreteModel* model)
    { return this->Superclass::AbleToOperate(model); }

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModelWrapper* ModelWrapper);

private:
  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  vtkMergeOperator(const vtkMergeOperator&);  // Not implemented.
  void operator=(const vtkMergeOperator&);  // Not implemented.
};

#endif
