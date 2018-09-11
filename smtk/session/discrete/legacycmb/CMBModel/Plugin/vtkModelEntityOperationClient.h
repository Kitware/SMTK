//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelEntityOperationClient - Change properties of model entities.
// .SECTION Description
// Operation to change the color (RGBA), the user name and/or visibility of a
// vtkModelEntity on the client.

#ifndef __vtkModelEntityOperationClient_h
#define __vtkModelEntityOperationClient_h

#include "cmbSystemConfig.h"
#include "vtkModelEntityOperationBase.h"

class vtkDiscreteModel;
class vtkModelEntity;
class vtkSMProxy;

class VTK_EXPORT vtkModelEntityOperationClient : public vtkModelEntityOperationBase
{
public:
  static vtkModelEntityOperationClient* New();
  vtkTypeMacro(vtkModelEntityOperationClient, vtkModelEntityOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  using Superclass::Operate;

  // Description:
  // Modify the color, username, and/or the visibility of an object.
  // Returns true if the operation completed successfully.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

protected:
  vtkModelEntityOperationClient();
  virtual ~vtkModelEntityOperationClient();

private:
  vtkModelEntityOperationClient(const vtkModelEntityOperationClient&); // Not implemented.
  void operator=(const vtkModelEntityOperationClient&);                // Not implemented.
};

#endif
