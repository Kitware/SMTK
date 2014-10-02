//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelBuilder2DClient - The client side object of vtkCMBModelBuilder2D
// .SECTION Description
// This class is a client counter-part for a GenerateSimpleModelOperator
// which is used to generate a 2-D model from a set of triangles with
// optional "modelfaceids" cell-data.
//
// It is responsible for setting up and getting result from the server side
// CMB operator.

#ifndef __vtkCMBModelBuilder2DClient_h
#define __vtkCMBModelBuilder2DClient_h

#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkSMProxy;

class VTK_EXPORT vtkCMBModelBuilder2DClient : public vtkObject
{
public:
  static vtkCMBModelBuilder2DClient * New();
  vtkTypeMacro(vtkCMBModelBuilder2DClient,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Using the input poly on the server to set up the modle
  // and serializes the model to the client.
  // Returns true if the operation was successful.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ModelWrapper,
                       vtkSMProxy* PolySourceProxy, int cleanVerts);

  // Description:
  // Static function to serialize an entire model from the server
  // to the client.  Returns true if the update was successful.
  static bool UpdateClientModel(vtkDiscreteModel* ClientModel,
                                vtkSMProxy* ModelWrapper);

protected:
  vtkCMBModelBuilder2DClient();
  virtual ~vtkCMBModelBuilder2DClient();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

private:

  vtkCMBModelBuilder2DClient(const vtkCMBModelBuilder2DClient&);  // Not implemented.
  void operator=(const vtkCMBModelBuilder2DClient&);  // Not implemented.
};

#endif
