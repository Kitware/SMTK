//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkEdgeSplitOperationClient - Split a model edge
// .SECTION Description
// Operation to split a model edge given a point id for the grid.  This
// will trigger the split operation on the server first and if it's
// successful it will do the same on the client. Note
// that the master polydata and the model edge polydata share the
// same point set so there is no concern about getting the point Ids
// confused.

#ifndef __vtkEdgeSplitOperationClient_h
#define __vtkEdgeSplitOperationClient_h

#include "cmbSystemConfig.h"
#include "vtkEdgeSplitOperationBase.h"

class vtkDiscreteModel;
class vtkSMProxy;

class VTK_EXPORT vtkEdgeSplitOperationClient : public vtkEdgeSplitOperationBase
{
public:
  static vtkEdgeSplitOperationClient* New();
  vtkTypeMacro(vtkEdgeSplitOperationClient, vtkEdgeSplitOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Initiate the split on the server and then follow up with the
  // split on the client.
  virtual bool Operate(vtkDiscreteModel* model, vtkSMProxy* serverModelProxy);

protected:
  vtkEdgeSplitOperationClient();
  virtual ~vtkEdgeSplitOperationClient();

private:
  vtkEdgeSplitOperationClient(const vtkEdgeSplitOperationClient&); // Not implemented.
  void operator=(const vtkEdgeSplitOperationClient&);              // Not implemented.
};

#endif
