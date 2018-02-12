//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelEdgeOperationClient - Change properties of model entities.
// .SECTION Description
// Operation to change line resolution, the color (RGBA), the user name
// and/or visibility of a vtkModelEntity on the client.

#ifndef __vtkModelEdgeOperationClient_h
#define __vtkModelEdgeOperationClient_h

#include "cmbSystemConfig.h"
#include "vtkModelEdgeOperationBase.h"

class vtkDiscreteModel;
class vtkModelEntity;
class vtkSMProxy;

class VTK_EXPORT vtkModelEdgeOperationClient : public vtkModelEdgeOperationBase
{
public:
  static vtkModelEdgeOperationClient* New();
  vtkTypeMacro(vtkModelEdgeOperationClient, vtkModelEdgeOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  using Superclass::Operate;

  // Description:
  // Modify the color, username, and/or the visibility of an object.
  // Returns true if the operation completed successfully.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

protected:
  vtkModelEdgeOperationClient();
  virtual ~vtkModelEdgeOperationClient();

private:
  vtkModelEdgeOperationClient(const vtkModelEdgeOperationClient&); // Not implemented.
  void operator=(const vtkModelEdgeOperationClient&);              // Not implemented.
};

#endif
