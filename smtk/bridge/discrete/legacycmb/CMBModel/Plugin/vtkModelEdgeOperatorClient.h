//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelEdgeOperatorClient - Change properties of model entities.
// .SECTION Description
// Operator to change line resolution, the color (RGBA), the user name
// and/or visibility of a vtkModelEntity on the client.

#ifndef __vtkModelEdgeOperatorClient_h
#define __vtkModelEdgeOperatorClient_h

#include "cmbSystemConfig.h"
#include "vtkModelEdgeOperatorBase.h"

class vtkDiscreteModel;
class vtkModelEntity;
class vtkSMProxy;

class VTK_EXPORT vtkModelEdgeOperatorClient : public vtkModelEdgeOperatorBase
{
public:
  static vtkModelEdgeOperatorClient * New();
  vtkTypeMacro(vtkModelEdgeOperatorClient,vtkModelEdgeOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  using Superclass::Operate;

  // Description:
  // Modify the color, username, and/or the visibility of an object.
  // Returns true if the operation completed successfully.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

protected:
  vtkModelEdgeOperatorClient();
  virtual ~vtkModelEdgeOperatorClient();

private:
  vtkModelEdgeOperatorClient(const vtkModelEdgeOperatorClient&);  // Not implemented.
  void operator=(const vtkModelEdgeOperatorClient&);  // Not implemented.
};

#endif
