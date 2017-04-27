//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelStateOperatorClient - client side equivalent of vtkCMBModelStateOperator
// .SECTION Description
//  This class is a client side convenient class to handle setting up and
//  getting result from server side vtkCMBModelStateOperator

#ifndef __vtkCMBModelStateOperatorClient_h
#define __vtkCMBModelStateOperatorClient_h

#include "cmbSystemConfig.h"
#include "vtkCMBModelStateOperatorBase.h"

class vtkDiscreteModelWrapper;
class vtkSMOperatorProxy;
class vtkSMProxy;

class VTK_EXPORT vtkCMBModelStateOperatorClient : public vtkCMBModelStateOperatorBase
{
public:
  static vtkCMBModelStateOperatorClient* New();
  vtkTypeMacro(vtkCMBModelStateOperatorClient, vtkCMBModelStateOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Save and Reload model state
  virtual int SaveModelState(vtkDiscreteModel* clientModel, vtkSMProxy* serverModelProxy);
  virtual int LoadModelState(vtkDiscreteModel* clientModel, vtkSMProxy* serverModelProxy);

  virtual vtkStringArray* GetSerializedModelString();

protected:
  vtkCMBModelStateOperatorClient();
  ~vtkCMBModelStateOperatorClient();

private:
  vtkCMBModelStateOperatorClient(const vtkCMBModelStateOperatorClient&); // Not implemented.
  void operator=(const vtkCMBModelStateOperatorClient&);                 // Not implemented.

  // Description:
  // Proxy for the server side operator
  vtkSMOperatorProxy* OperatorProxy;
};

#endif
