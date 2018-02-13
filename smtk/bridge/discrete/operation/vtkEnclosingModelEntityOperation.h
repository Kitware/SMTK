//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkEnclosingModelEntityOperation -
// .SECTION Description

#ifndef __vtkEnclosingModelEntityOperation_h
#define __vtkEnclosingModelEntityOperation_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkObject.h"

class vtkDiscreteModelWrapper;
//class vtkIdTypeArray;
class vtkModelEntity;
class vtkCellLocator;

class SMTKDISCRETESESSION_EXPORT vtkEnclosingModelEntityOperation : public vtkObject
{
public:
  static vtkEnclosingModelEntityOperation* New();
  vtkTypeMacro(vtkEnclosingModelEntityOperation, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void BuildLinks(vtkDiscreteModelWrapper* modelWrapper);

  // Description:
  // Modify the color and/or the visibility of an object.
  virtual void Operate(vtkDiscreteModelWrapper* modelWrapper, double* pt);

  // Description:
  // Return the model entity.
  vtkGetObjectMacro(EnclosingEntity, vtkModelEntity);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkEnclosingModelEntityOperation();
  ~vtkEnclosingModelEntityOperation() override;

private:
  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  vtkCellLocator* CellLocator;
  vtkModelEntity* EnclosingEntity;

  vtkEnclosingModelEntityOperation(const vtkEnclosingModelEntityOperation&); // Not implemented.
  void operator=(const vtkEnclosingModelEntityOperation&);                   // Not implemented.
};

#endif
