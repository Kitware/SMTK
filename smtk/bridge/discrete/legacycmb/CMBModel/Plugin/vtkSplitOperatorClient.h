//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkSplitOperatorClient - Split a model face on the client
// .SECTION Description
// Operator to split a model face given an angle on the client.
// This will also perform the split operation on the server.

#ifndef __vtkSplitOperatorClient_h
#define __vtkSplitOperatorClient_h

#include "vtkSplitOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;
class vtkIdTypeArray;
class vtkModelEntity;
class vtkSMProxy;

class VTK_EXPORT vtkSplitOperatorClient : public vtkSplitOperatorBase
{
public:
  static vtkSplitOperatorClient * New();
  vtkTypeMacro(vtkSplitOperatorClient,vtkSplitOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

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
  vtkSplitOperatorClient();
  virtual ~vtkSplitOperatorClient();

private:
  vtkSplitOperatorClient(const vtkSplitOperatorClient&);  // Not implemented.
  void operator=(const vtkSplitOperatorClient&);  // Not implemented.
};

#endif
