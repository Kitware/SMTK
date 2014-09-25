/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME vtkCMBModelStateOperatorClient - client side equivalent of vtkCMBModelStateOperator
// .SECTION Description
//  This class is a client side convenient class to handle setting up and
//  getting result from server side vtkCMBModelStateOperator

#ifndef __vtkCMBModelStateOperatorClient_h
#define __vtkCMBModelStateOperatorClient_h

#include "vtkCMBModelStateOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;
class vtkSMOperatorProxy;
class vtkSMProxy;

class VTK_EXPORT vtkCMBModelStateOperatorClient : public vtkCMBModelStateOperatorBase
{
public:
  static vtkCMBModelStateOperatorClient *New();
  vtkTypeMacro(vtkCMBModelStateOperatorClient,vtkCMBModelStateOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Save and Reload model state
  virtual int SaveModelState(vtkDiscreteModel *clientModel, vtkSMProxy* serverModelProxy);
  virtual int LoadModelState(vtkDiscreteModel *clientModel, vtkSMProxy* serverModelProxy);

  virtual vtkStringArray* GetSerializedModelString();

protected:
  vtkCMBModelStateOperatorClient();
  ~vtkCMBModelStateOperatorClient();

private:
  vtkCMBModelStateOperatorClient(const vtkCMBModelStateOperatorClient&);  // Not implemented.
  void operator=(const vtkCMBModelStateOperatorClient&);  // Not implemented.

  // Description:
  // Proxy for the server side operator
  vtkSMOperatorProxy* OperatorProxy;

};

#endif
