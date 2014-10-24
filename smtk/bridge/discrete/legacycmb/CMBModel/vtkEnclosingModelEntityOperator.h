//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkEnclosingModelEntityOperator -
// .SECTION Description


#ifndef __vtkEnclosingModelEntityOperator_h
#define __vtkEnclosingModelEntityOperator_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;
//class vtkIdTypeArray;
class vtkModelEntity;
class vtkCellLocator;

class VTKCMBDISCRETEMODEL_EXPORT vtkEnclosingModelEntityOperator : public vtkObject
{
public:
  static vtkEnclosingModelEntityOperator * New();
  vtkTypeMacro(vtkEnclosingModelEntityOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  void BuildLinks(vtkDiscreteModelWrapper* modelWrapper);

  // Description:
  // Modify the color and/or the visibility of an object.
  virtual void Operate(vtkDiscreteModelWrapper* modelWrapper, double *pt);

  // Description:
  // Return the model entity.
  vtkGetObjectMacro(EnclosingEntity, vtkModelEntity);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkEnclosingModelEntityOperator();
  virtual ~vtkEnclosingModelEntityOperator();

private:
  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  vtkCellLocator *CellLocator;
  vtkModelEntity *EnclosingEntity;

  vtkEnclosingModelEntityOperator(const vtkEnclosingModelEntityOperator&);  // Not implemented.
  void operator=(const vtkEnclosingModelEntityOperator&);  // Not implemented.
};

#endif
