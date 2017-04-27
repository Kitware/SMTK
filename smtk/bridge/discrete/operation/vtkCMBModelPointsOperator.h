//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelPointsOperator -
// .SECTION Description
// An operator to only SetPoints for model geometry, and the cell structure
// is unchanged.

#ifndef __smtkdiscrete_vtkCMBModelPointsOperator_h
#define __smtkdiscrete_vtkCMBModelPointsOperator_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkObject.h"

class vtkDiscreteModelWrapper;
class vtkPointSet;
class vtkPointData;
class vtkAlgorithm;

class SMTKDISCRETESESSION_EXPORT vtkCMBModelPointsOperator : public vtkObject
{
public:
  static vtkCMBModelPointsOperator* New();
  vtkTypeMacro(vtkCMBModelPointsOperator, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

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
  vtkCMBModelPointsOperator();
  virtual ~vtkCMBModelPointsOperator();

private:
  vtkCMBModelPointsOperator(const vtkCMBModelPointsOperator&); // Not implemented.
  void operator=(const vtkCMBModelPointsOperator&);            // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
  vtkPointSet* ModelPoints;
  vtkPointData* ModelPointData;
};

#endif
