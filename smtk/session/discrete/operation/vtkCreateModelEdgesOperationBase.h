//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCreateModelEdgesOperationBase - Creates the model edges of a model
// .SECTION Description
// Operation to extract the model edges of a model that also consists of model regions

#ifndef __smtkdiscrete_vtkCreateModelEdgesOperationBase_h
#define __smtkdiscrete_vtkCreateModelEdgesOperationBase_h

#include "smtk/session/discrete/Exports.h" // For export macro
#include "vtkObject.h"

class vtkDiscreteModel;

class SMTKDISCRETESESSION_EXPORT vtkCreateModelEdgesOperationBase : public vtkObject
{
public:
  static vtkCreateModelEdgesOperationBase* New();
  vtkTypeMacro(vtkCreateModelEdgesOperationBase, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkCreateModelEdgesOperationBase();
  ~vtkCreateModelEdgesOperationBase() override;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

private:
  vtkCreateModelEdgesOperationBase(const vtkCreateModelEdgesOperationBase&); // Not implemented.
  void operator=(const vtkCreateModelEdgesOperationBase&);                   // Not implemented.
};

#endif
