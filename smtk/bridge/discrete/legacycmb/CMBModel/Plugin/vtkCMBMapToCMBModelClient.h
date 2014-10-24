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

#ifndef __vtkCMBMapToCMBModelClient_h
#define __vtkCMBMapToCMBModelClient_h

#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkSMProxy;

class VTK_EXPORT vtkCMBMapToCMBModelClient : public vtkObject
{
  public:
    static vtkCMBMapToCMBModelClient * New();
    vtkTypeMacro(vtkCMBMapToCMBModelClient,vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Using the input poly on the server to set up the modle
    // and serializes the model to the client.
    // Returns true if the operation was successful.
    virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy,
        vtkSMProxy* PolySourceProxy);

  protected:
    vtkCMBMapToCMBModelClient();
    virtual ~vtkCMBMapToCMBModelClient();

    // Description:
    // Check to see if everything is properly set for the operator.
    virtual bool AbleToOperate(vtkDiscreteModel* Model);

  private:

    vtkCMBMapToCMBModelClient(const vtkCMBMapToCMBModelClient&);  // Not implemented.
    void operator=(const vtkCMBMapToCMBModelClient&);  // Not implemented.
};

#endif
