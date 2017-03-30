//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCreateModelEdgesOperatorClient - Split a model face on the client
// .SECTION Description
// Operator to split a model face given an angle on the client.
// This will also perform the split operation on the server.

#ifndef __vtkCreateModelEdgesOperatorClient_h
#define __vtkCreateModelEdgesOperatorClient_h

#include "cmbSystemConfig.h"
#include "vtkCreateModelEdgesOperatorBase.h"

class vtkDiscreteModelWrapper;
class vtkSMProxy;
class vtkDiscreteModel;
class vtkStringArray;

class VTK_EXPORT vtkCreateModelEdgesOperatorClient : public vtkCreateModelEdgesOperatorBase
{
public:
  static vtkCreateModelEdgesOperatorClient* New();
  vtkTypeMacro(vtkCreateModelEdgesOperatorClient, vtkCreateModelEdgesOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Modify the color and/or the visibility of an object. The
  // operator returns true for successful completion.
  virtual bool Operate(
    vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy, const char* strSerializedModel);

protected:
  vtkCreateModelEdgesOperatorClient();
  virtual ~vtkCreateModelEdgesOperatorClient();

private:
  vtkCreateModelEdgesOperatorClient(const vtkCreateModelEdgesOperatorClient&); // Not implemented.
  void operator=(const vtkCreateModelEdgesOperatorClient&);                    // Not implemented.
};

#endif
