//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkMergeOperatorClient - Merge a set of geometric model entities
// .SECTION Description
// Operator to merge a set of source geometric model entities into
// a target geometric entity on the client.  It also calls the operator
// on the server.  The properties of the target entity
// (e.g. color, BCS/ModelEntityGroup associations) will not be changed.
// Warning: This may only currently work from model faces.

#ifndef __vtkMergeOperatorClient_h
#define __vtkMergeOperatorClient_h

#include "cmbSystemConfig.h"
#include "vtkMergeOperatorBase.h"

class vtkDiscreteModel;
class vtkSMProxy;

class VTK_EXPORT vtkMergeOperatorClient : public vtkMergeOperatorBase
{
public:
  static vtkMergeOperatorClient * New();
  vtkTypeMacro(vtkMergeOperatorClient,vtkMergeOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  using Superclass::Operate;

  // Description:
  // Merge the specified geometric model entities on both the client
  // and the server.
  // Returns true if the operation completed successfully.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

protected:
  vtkMergeOperatorClient();
  virtual ~vtkMergeOperatorClient();

private:
  vtkMergeOperatorClient(const vtkMergeOperatorClient&);  // Not implemented.
  void operator=(const vtkMergeOperatorClient&);  // Not implemented.
};

#endif
