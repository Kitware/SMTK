//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkSMOperationProxy
// .SECTION Description
//

#ifndef __vtkSMOperationProxy_h
#define __vtkSMOperationProxy_h

#include "cmbSystemConfig.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkSMProxy.h"

class vtkDiscreteModel;

class VTKCMBDISCRETEMODEL_EXPORT vtkSMOperationProxy : public vtkSMProxy
{
public:
  static vtkSMOperationProxy* New();
  vtkTypeMacro(vtkSMOperationProxy, vtkSMProxy);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Operate on the model on the server.
  virtual void Operate(vtkDiscreteModel* ClientModel, vtkSMProxy* ModelProxy);

  // Description:
  // Operate on the model on the server with a given input proxy.
  virtual void Operate(
    vtkDiscreteModel* ClientModel, vtkSMProxy* ModelProxy, vtkSMProxy* InputProxy);

  // Description:
  // Build an object on the model on the server.
  virtual vtkIdType Build(vtkDiscreteModel* ClientModel, vtkSMProxy* ModelProxy);

  // Description:
  // Destroy, if possible, an object on the model on the server.
  virtual bool Destroy(vtkDiscreteModel* ClientModel, vtkSMProxy* ModelProxy);

protected:
  vtkSMOperationProxy();
  ~vtkSMOperationProxy();

private:
  vtkSMOperationProxy(const vtkSMOperationProxy&); // Not implemented
  void operator=(const vtkSMOperationProxy&);      // Not implemented
};

#endif
