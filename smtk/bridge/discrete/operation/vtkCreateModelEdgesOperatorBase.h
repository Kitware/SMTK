//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCreateModelEdgesOperatorBase - Creates the model edges of a model
// .SECTION Description
// Operator to extract the model edges of a model that also consists of model regions

#ifndef __smtkdiscrete_vtkCreateModelEdgesOperatorBase_h
#define __smtkdiscrete_vtkCreateModelEdgesOperatorBase_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkObject.h"


class vtkDiscreteModel;

class SMTKDISCRETESESSION_EXPORT vtkCreateModelEdgesOperatorBase : public vtkObject
{
public:
  static vtkCreateModelEdgesOperatorBase * New();
  vtkTypeMacro(vtkCreateModelEdgesOperatorBase,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkCreateModelEdgesOperatorBase();
  virtual ~vtkCreateModelEdgesOperatorBase();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

private:

  vtkCreateModelEdgesOperatorBase(const vtkCreateModelEdgesOperatorBase&);  // Not implemented.
  void operator=(const vtkCreateModelEdgesOperatorBase&);  // Not implemented.
};

#endif
