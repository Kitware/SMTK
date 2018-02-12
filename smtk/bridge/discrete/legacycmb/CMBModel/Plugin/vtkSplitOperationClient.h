//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkSplitOperationClient - Split a model face on the client
// .SECTION Description
// Operation to split a model face given an angle on the client.
// This will also perform the split operation on the server.

#ifndef __vtkSplitOperationClient_h
#define __vtkSplitOperationClient_h

#include "cmbSystemConfig.h"
#include "vtkSplitOperationBase.h"

class vtkDiscreteModelWrapper;
class vtkIdTypeArray;
class vtkModelEntity;
class vtkSMProxy;

class VTK_EXPORT vtkSplitOperationClient : public vtkSplitOperationBase
{
public:
  static vtkSplitOperationClient* New();
  vtkTypeMacro(vtkSplitOperationClient, vtkSplitOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Modify the color and/or the visibility of an object. The
  // operator returns true for successful completion.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

  // Description:
  // This is edge splitting info for splitting hybrid model faces.
  // <ExistingEdgeId, [NewEdgeIds] >
  // <ExistingEdgeId, [NewVertIds] >
  std::map<vtkIdType, std::vector<vtkIdType> > SplitEdgeMap;
  std::map<vtkIdType, std::vector<vtkIdType> > SplitVertMap;
  std::vector<vtkIdType> NewEdges; // not associated with existing edge
  std::vector<vtkIdType> NewVerts; // not associated with existing edge

protected:
  vtkSplitOperationClient();
  virtual ~vtkSplitOperationClient();

private:
  vtkSplitOperationClient(const vtkSplitOperationClient&); // Not implemented.
  void operator=(const vtkSplitOperationClient&);          // Not implemented.
};

#endif
