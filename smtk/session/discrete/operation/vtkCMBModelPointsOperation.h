//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelPointsOperation -
// .SECTION Description
// An operator to only SetPoints for model geometry, and the cell structure
// is unchanged.

#ifndef __smtkdiscrete_vtkCMBModelPointsOperation_h
#define __smtkdiscrete_vtkCMBModelPointsOperation_h

#include "smtk/session/discrete/Exports.h" // For export macro
#include "vtkObject.h"

class vtkDiscreteModelWrapper;
class vtkPointSet;
class vtkPointData;
class vtkAlgorithm;

class SMTKDISCRETESESSION_EXPORT vtkCMBModelPointsOperation : public vtkObject
{
public:
  static vtkCMBModelPointsOperation* New();
  vtkTypeMacro(vtkCMBModelPointsOperation, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Load the file into Model.
  void Operate(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Set model points
  void SetModelPointsInput(vtkAlgorithm* dataAlg);
  void SetModelPoints(vtkPointSet* dataObj);
  vtkGetObjectMacro(ModelPoints, vtkPointSet);

  // Description:
  // Set model points
  void SetModelPointDataInput(vtkAlgorithm* dataAlg);
  void SetModelPointData(vtkPointData* pointData);
  vtkGetObjectMacro(ModelPointData, vtkPointData);

  // Description:
  // Returns success (1) or failure (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkCMBModelPointsOperation();
  ~vtkCMBModelPointsOperation() override;

private:
  vtkCMBModelPointsOperation(const vtkCMBModelPointsOperation&); // Not implemented.
  void operator=(const vtkCMBModelPointsOperation&);             // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
  vtkPointSet* ModelPoints;
  vtkPointData* ModelPointData;
};

#endif
