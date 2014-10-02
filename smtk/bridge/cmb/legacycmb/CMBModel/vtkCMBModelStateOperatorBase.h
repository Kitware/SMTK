//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelStateOperatorBase - Base class for the model state operator classes.
// .SECTION Description
//  This is a base class for the state operator classes,
//  vtkCMBModelStateOperator and vtkCMBModelStateOperatorClient

#ifndef __vtkCMBModelStateOperatorBase_h
#define __vtkCMBModelStateOperatorBase_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include <map>
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;
class vtkStringArray;
class vtkIdList;
class vtkDiscreteModel;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBModelStateOperatorBase : public vtkObject
{
public:
  static vtkCMBModelStateOperatorBase *New();
  vtkTypeMacro(vtkCMBModelStateOperatorBase,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the serialized string of the model.
  virtual vtkStringArray* GetSerializedModelString()
    {return this->SerializedModelString;}

protected:
  vtkCMBModelStateOperatorBase();
  ~vtkCMBModelStateOperatorBase();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

  // Internal convenient ivars.
  vtkStringArray *SerializedModelString;

private:
  vtkCMBModelStateOperatorBase(const vtkCMBModelStateOperatorBase&);  // Not implemented.
  void operator=(const vtkCMBModelStateOperatorBase&);  // Not implemented.
};

#endif
