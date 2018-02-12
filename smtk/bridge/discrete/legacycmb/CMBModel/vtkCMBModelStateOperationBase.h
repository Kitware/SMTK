//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelStateOperationBase - Base class for the model state operator classes.
// .SECTION Description
//  This is a base class for the state operator classes,
//  vtkCMBModelStateOperation and vtkCMBModelStateOperationClient

#ifndef __vtkCMBModelStateOperationBase_h
#define __vtkCMBModelStateOperationBase_h

#include "cmbSystemConfig.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include <map>

class vtkDiscreteModelWrapper;
class vtkStringArray;
class vtkIdList;
class vtkDiscreteModel;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBModelStateOperationBase : public vtkObject
{
public:
  static vtkCMBModelStateOperationBase* New();
  vtkTypeMacro(vtkCMBModelStateOperationBase, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Get the serialized string of the model.
  virtual vtkStringArray* GetSerializedModelString() { return this->SerializedModelString; }

protected:
  vtkCMBModelStateOperationBase();
  ~vtkCMBModelStateOperationBase();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

  // Internal convenient ivars.
  vtkStringArray* SerializedModelString;

private:
  vtkCMBModelStateOperationBase(const vtkCMBModelStateOperationBase&); // Not implemented.
  void operator=(const vtkCMBModelStateOperationBase&);                // Not implemented.
};

#endif
