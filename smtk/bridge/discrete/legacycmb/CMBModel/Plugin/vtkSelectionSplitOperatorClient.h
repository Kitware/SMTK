//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkSelectionSplitOperatorClient - Split a model face based on a vtkSelection.
// .SECTION Description
// Operator to split model faces based on a vtkSelection. Note
// that the cell Ids included in the vtkSelection are with respect
// to the master vtkPolyData.  This is the interface for splitting
// the model face on the client.

#ifndef __vtkSelectionSplitOperatorClient_h
#define __vtkSelectionSplitOperatorClient_h

#include "vtkSelectionSplitOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkIdTypeArray;
class vtkSMProxy;
class vtkSMIdTypeVectorProperty;

class VTK_EXPORT vtkSelectionSplitOperatorClient : public vtkSelectionSplitOperatorBase
{
public:
  static vtkSelectionSplitOperatorClient * New();
  vtkTypeMacro(vtkSelectionSplitOperatorClient,vtkSelectionSplitOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Split the model faces based on the input Selection.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy,
                       vtkSMProxy* SelectionSourceProxy);

  // Description:
  // Update Client model after face split on server with hybrid model
  static void UpdateSplitEdgeVertIds(
      vtkDiscreteModel* Model, vtkSMIdTypeVectorProperty* SplitEdgeVertIds,
      std::map<vtkIdType, std::vector<vtkIdType> >& SplitEdgeMap,
      std::map<vtkIdType, std::vector<vtkIdType> >& SplitVertMap);
  static void UpdateCreatedModelEdgeVertIDs(
      vtkDiscreteModel* Model, vtkSMIdTypeVectorProperty* CreatedModelEdgeVertIDs,
      std::vector<vtkIdType>& newEdges, std::vector<vtkIdType>& newVerts);
  static void UpdateFaceEdgeLoopIDs(
      vtkDiscreteModel* Model, vtkSMIdTypeVectorProperty* FaceEdgeLoopIDs);

  // Description:
  // This is edge splitting info for splitting hybrid model faces.
  // <ExistingEdgeId, [NewEdgeIds] >
  // <ExistingEdgeId, [NewVertIds] >
  std::map<vtkIdType, std::vector<vtkIdType> > SplitEdgeMap;
  std::map<vtkIdType, std::vector<vtkIdType> > SplitVertMap;
  std::vector<vtkIdType> NewEdges; // not associated with existing edge
  std::vector<vtkIdType> NewVerts; // not associated with existing edge

protected:
  vtkSelectionSplitOperatorClient();
  virtual ~vtkSelectionSplitOperatorClient();

private:
  vtkSelectionSplitOperatorClient(const vtkSelectionSplitOperatorClient&);  // Not implemented.
  void operator=(const vtkSelectionSplitOperatorClient&);  // Not implemented.
};

#endif
