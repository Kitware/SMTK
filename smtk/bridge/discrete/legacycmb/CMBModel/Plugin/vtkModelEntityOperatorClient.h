//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelEntityOperatorClient - Change properties of model entities.
// .SECTION Description
// Operator to change the color (RGBA), the user name and/or visibility of a
// vtkModelEntity on the client.

#ifndef __vtkModelEntityOperatorClient_h
#define __vtkModelEntityOperatorClient_h

#include "cmbSystemConfig.h"
#include "vtkModelEntityOperatorBase.h"

class vtkDiscreteModel;
class vtkModelEntity;
class vtkSMProxy;

class VTK_EXPORT vtkModelEntityOperatorClient : public vtkModelEntityOperatorBase
{
public:
  static vtkModelEntityOperatorClient* New();
  vtkTypeMacro(vtkModelEntityOperatorClient, vtkModelEntityOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  using Superclass::Operate;

  // Description:
  // Modify the color, username, and/or the visibility of an object.
  // Returns true if the operation completed successfully.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

protected:
  vtkModelEntityOperatorClient();
  virtual ~vtkModelEntityOperatorClient();

private:
  vtkModelEntityOperatorClient(const vtkModelEntityOperatorClient&); // Not implemented.
  void operator=(const vtkModelEntityOperatorClient&);               // Not implemented.
};

#endif
