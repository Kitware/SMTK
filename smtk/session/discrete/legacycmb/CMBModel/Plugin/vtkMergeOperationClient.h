//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkMergeOperationClient - Merge a set of geometric model entities
// .SECTION Description
// Operation to merge a set of source geometric model entities into
// a target geometric entity on the client.  It also calls the operator
// on the server.  The properties of the target entity
// (e.g. color, BCS/ModelEntityGroup associations) will not be changed.
// Warning: This may only currently work from model faces.

#ifndef __vtkMergeOperationClient_h
#define __vtkMergeOperationClient_h

#include "cmbSystemConfig.h"
#include "vtkMergeOperationBase.h"

class vtkDiscreteModel;
class vtkSMProxy;

class VTK_EXPORT vtkMergeOperationClient : public vtkMergeOperationBase
{
public:
  static vtkMergeOperationClient* New();
  vtkTypeMacro(vtkMergeOperationClient, vtkMergeOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  using Superclass::Operate;

  // Description:
  // Merge the specified geometric model entities on both the client
  // and the server.
  // Returns true if the operation completed successfully.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

protected:
  vtkMergeOperationClient();
  virtual ~vtkMergeOperationClient();

private:
  vtkMergeOperationClient(const vtkMergeOperationClient&); // Not implemented.
  void operator=(const vtkMergeOperationClient&);          // Not implemented.
};

#endif
