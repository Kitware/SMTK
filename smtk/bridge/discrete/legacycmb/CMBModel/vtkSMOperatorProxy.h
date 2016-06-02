//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkSMOperatorProxy
// .SECTION Description
//

#ifndef __vtkSMOperatorProxy_h
#define __vtkSMOperatorProxy_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkSMProxy.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;

class VTKCMBDISCRETEMODEL_EXPORT vtkSMOperatorProxy : public vtkSMProxy
{
public:
  static vtkSMOperatorProxy* New();
  vtkTypeMacro(vtkSMOperatorProxy, vtkSMProxy);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Operate on the model on the server.
  virtual void Operate(vtkDiscreteModel* ClientModel, vtkSMProxy* ModelProxy);

  // Description:
  // Operate on the model on the server with a given input proxy.
  virtual void Operate(vtkDiscreteModel* ClientModel, vtkSMProxy* ModelProxy,
                       vtkSMProxy* InputProxy);

  // Description:
  // Build an object on the model on the server.
  virtual vtkIdType Build(vtkDiscreteModel* ClientModel, vtkSMProxy* ModelProxy);

  // Description:
  // Destroy, if possible, an object on the model on the server.
  virtual bool Destroy(vtkDiscreteModel* ClientModel, vtkSMProxy* ModelProxy);

protected:
  vtkSMOperatorProxy();
  ~vtkSMOperatorProxy();

private:
  vtkSMOperatorProxy(const vtkSMOperatorProxy&); // Not implemented
  void operator=(const vtkSMOperatorProxy&); // Not implemented

};

#endif

