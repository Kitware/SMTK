//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCreateModelEdgesOperationClient - Split a model face on the client
// .SECTION Description
// Operation to split a model face given an angle on the client.
// This will also perform the split operation on the server.

#ifndef __vtkCreateModelEdgesOperationClient_h
#define __vtkCreateModelEdgesOperationClient_h

#include "cmbSystemConfig.h"
#include "vtkCreateModelEdgesOperationBase.h"

class vtkDiscreteModelWrapper;
class vtkSMProxy;
class vtkDiscreteModel;
class vtkStringArray;

class VTK_EXPORT vtkCreateModelEdgesOperationClient : public vtkCreateModelEdgesOperationBase
{
public:
  static vtkCreateModelEdgesOperationClient* New();
  vtkTypeMacro(vtkCreateModelEdgesOperationClient, vtkCreateModelEdgesOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Modify the color and/or the visibility of an object. The
  // operator returns true for successful completion.
  virtual bool Operate(
    vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy, const char* strSerializedModel);

protected:
  vtkCreateModelEdgesOperationClient();
  virtual ~vtkCreateModelEdgesOperationClient();

private:
  vtkCreateModelEdgesOperationClient(const vtkCreateModelEdgesOperationClient&); // Not implemented.
  void operator=(const vtkCreateModelEdgesOperationClient&);                     // Not implemented.
};

#endif
