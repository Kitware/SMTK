//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelStateOperationClient - client side equivalent of vtkCMBModelStateOperation
// .SECTION Description
//  This class is a client side convenient class to handle setting up and
//  getting result from server side vtkCMBModelStateOperation

#ifndef __vtkCMBModelStateOperationClient_h
#define __vtkCMBModelStateOperationClient_h

#include "cmbSystemConfig.h"
#include "vtkCMBModelStateOperationBase.h"

class vtkDiscreteModelWrapper;
class vtkSMOperationProxy;
class vtkSMProxy;

class VTK_EXPORT vtkCMBModelStateOperationClient : public vtkCMBModelStateOperationBase
{
public:
  static vtkCMBModelStateOperationClient* New();
  vtkTypeMacro(vtkCMBModelStateOperationClient, vtkCMBModelStateOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Save and Reload model state
  virtual int SaveModelState(vtkDiscreteModel* clientModel, vtkSMProxy* serverModelProxy);
  virtual int LoadModelState(vtkDiscreteModel* clientModel, vtkSMProxy* serverModelProxy);

  virtual vtkStringArray* GetSerializedModelString();

protected:
  vtkCMBModelStateOperationClient();
  ~vtkCMBModelStateOperationClient();

private:
  vtkCMBModelStateOperationClient(const vtkCMBModelStateOperationClient&); // Not implemented.
  void operator=(const vtkCMBModelStateOperationClient&);                  // Not implemented.

  // Description:
  // Proxy for the server side operator
  vtkSMOperationProxy* OperationProxy;
};

#endif
