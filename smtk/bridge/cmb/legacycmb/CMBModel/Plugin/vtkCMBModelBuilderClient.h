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
// .NAME vtkCMBModelBuilderClient - The client side object of vtkCMBModelBuilder
// .SECTION Description
// This class is the client counter-part of server side vtkCMBModelBulider.
// It is responsible for setting up and getting result from the server side
// CMB model builder.

#ifndef __vtkCMBModelBuilderClient_h
#define __vtkCMBModelBuilderClient_h

#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkSMProxy;

class VTK_EXPORT vtkCMBModelBuilderClient : public vtkObject
{
public:
  static vtkCMBModelBuilderClient * New();
  vtkTypeMacro(vtkCMBModelBuilderClient,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Using the input poly on the server to set up the modle
  // and serializes the model to the client.
  // Returns true if the operation was successful.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy,
                       vtkSMProxy* PolySourceProxy);

  // Description:
  // Static function to serialize an entire model from the server
  // to the client.  Returns true if the update was successful.
  static bool UpdateClientModel(vtkDiscreteModel* ClientModel,
                                vtkSMProxy* ServerModelProxy);

protected:
  vtkCMBModelBuilderClient();
  virtual ~vtkCMBModelBuilderClient();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

private:

  vtkCMBModelBuilderClient(const vtkCMBModelBuilderClient&);  // Not implemented.
  void operator=(const vtkCMBModelBuilderClient&);  // Not implemented.
};

#endif
