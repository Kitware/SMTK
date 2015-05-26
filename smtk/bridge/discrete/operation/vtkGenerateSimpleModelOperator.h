//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkGenerateSimpleModelOperator - create a simple model from polydata
// .SECTION Description
// Create a 2D model from the output from a filter that produces
// polydata. By default we assume that the model may have quads and/or
// bad topology (i.e. coincident points) so we use the triangle
// and clean polydata filter to "fix" it. The user can set CleanInput
// to 0 to avoid this overhead if the incoming polydata is "good".

#ifndef __smtkdiscrete_vtkGenerateSimpleModelOperator_h
#define __smtkdiscrete_vtkGenerateSimpleModelOperator_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkObject.h"

class vtkAlgorithm;
class vtkDiscreteModelWrapper;

class SMTKDISCRETESESSION_EXPORT vtkGenerateSimpleModelOperator : public vtkObject
{
public:
  static vtkGenerateSimpleModelOperator * New();
  vtkTypeMacro(vtkGenerateSimpleModelOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a model from the output from inputFilter. If cleanInput is
  // true then we also triangulate and remove coincident points.
  void Operate(vtkDiscreteModelWrapper* modelWrapper, vtkAlgorithm* inputFilter,
               int cleanInput);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkGenerateSimpleModelOperator();
  virtual ~vtkGenerateSimpleModelOperator();

private:
  vtkGenerateSimpleModelOperator(const vtkGenerateSimpleModelOperator&);  // Not implemented.
  void operator=(const vtkGenerateSimpleModelOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
