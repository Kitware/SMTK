//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkEdgeSplitOperationBase - Split a model edge
// .SECTION Description
// Operation to split a model edge given a point id for the grid.  Note
// that the master polydata and the model edge polydata share the
// same point set so there is no concern about getting the point Ids
// confused.

#ifndef __smtkdiscrete_vtkEdgeSplitOperationBase_h
#define __smtkdiscrete_vtkEdgeSplitOperationBase_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkObject.h"

class vtkDiscreteModel;
class vtkModelEntity;

class SMTKDISCRETESESSION_EXPORT vtkEdgeSplitOperationBase : public vtkObject
{
public:
  static vtkEdgeSplitOperationBase* New();
  vtkTypeMacro(vtkEdgeSplitOperationBase, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set/get the model entity unique persistent id to operate on.
  // This is required to be set before calling Operate.
  vtkGetMacro(IsEdgeIdSet, int);
  vtkGetMacro(EdgeId, vtkIdType);
  void SetEdgeId(vtkIdType id);

  // Description:
  // Set/get the point Id of where the model edge should be split at.
  // This is required to be set before calling Operate.
  vtkGetMacro(IsPointIdSet, int);
  vtkGetMacro(PointId, vtkIdType);
  void SetPointId(vtkIdType id);

  // Description:
  // Return the model entity.
  vtkModelEntity* GetModelEntity(vtkDiscreteModel* Model);

  // Description:
  // Get the Id of the created model edge during a split operation.
  vtkGetMacro(CreatedModelEdgeId, vtkIdType);

  // Description:
  // Get the Id of the created model vertex during a split operation.
  vtkGetMacro(CreatedModelVertexId, vtkIdType);

protected:
  vtkEdgeSplitOperationBase();
  ~vtkEdgeSplitOperationBase() override;

  // Description:
  // Set function for the created model edge id.  It is protected
  // since only the derived class should modify it.
  vtkSetMacro(CreatedModelEdgeId, vtkIdType);

  // Description:
  // Set function for the created model vertex id.  It is protected
  // since only the derived class should modify it.
  vtkSetMacro(CreatedModelVertexId, vtkIdType);

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

private:
  // Description:
  // The unique persistent Id of the model edge created (-1 if
  // no model edge was created).
  vtkIdType CreatedModelEdgeId;

  // Description:
  // The unique persistent Id of the model vertex created (-1 if
  // no model vertex was created).
  vtkIdType CreatedModelVertexId;

  // Description:
  // The unique persistent id of the model entity to be operated on.
  // This is required to be set before calling Operate.
  vtkIdType EdgeId;
  int IsEdgeIdSet;

  // Description:
  // The point id of the polydata where the model edge will be split.
  // This is required to be set before calling Operate.
  vtkIdType PointId;
  int IsPointIdSet;

  vtkEdgeSplitOperationBase(const vtkEdgeSplitOperationBase&); // Not implemented.
  void operator=(const vtkEdgeSplitOperationBase&);            // Not implemented.
};

#endif
