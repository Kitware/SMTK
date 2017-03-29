//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelBuilderClient - The client side object of vtkCMBModelBuilder
// .SECTION Description
// This class is the client counter-part of server side vtkCMBModelBulider.
// It is responsible for setting up and getting result from the server side
// CMB model builder.

#ifndef __vtkCMBModelBuilderClient_h
#define __vtkCMBModelBuilderClient_h

#include "cmbSystemConfig.h"
#include "vtkObject.h"

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
