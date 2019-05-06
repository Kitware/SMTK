//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkSplitOperationBase - Split a model face
// .SECTION Description
// Operation to split a model face given an angle.

#ifndef __smtkdiscrete_vtkSplitOperationBase_h
#define __smtkdiscrete_vtkSplitOperationBase_h

#include "smtk/session/discrete/Exports.h" // For export macro
#include "smtk/session/discrete/kernel/ModelEdgeHelper.h"
#include "vtkObject.h"

class vtkDiscreteModel;
class vtkIdTypeArray;
class vtkModelEntity;

class SMTKDISCRETESESSION_EXPORT vtkSplitOperationBase : public vtkObject
{
public:
  static vtkSplitOperationBase* New();
  vtkTypeMacro(vtkSplitOperationBase, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set/get the model entity unique persistent id to operate on.
  // This is required to be set before calling Operate.
  vtkGetMacro(IsIdSet, int);
  vtkGetMacro(Id, vtkIdType);
  void SetId(vtkIdType id);

  // Description:
  // Functions to set/get the split angle.
  vtkGetMacro(FeatureAngle, double);
  vtkGetMacro(IsFeatureAngleSet, int);
  void SetFeatureAngle(double featureAngle);

  // Description:
  // Return the model entity.
  vtkModelEntity* GetModelEntity(vtkDiscreteModel* Model);

  // Description:
  // Get the list of created model faces during a split operation.
  vtkGetObjectMacro(CreatedModelFaceIDs, vtkIdTypeArray);

  // Description:
  // Set/get the current new face Id, which is used to retrieve
  // the cached SplitInfo
  vtkGetMacro(CurrentNewFaceId, vtkIdType);
  vtkSetMacro(CurrentNewFaceId, vtkIdType);

  // Description:
  // Get FaceSplitInfo arrays AFTER a split operation, based on
  // CurrentNewFaceId
  virtual vtkIdTypeArray* GetSplitEdgeVertIds()
  {
    return this->GetSplitEdgeVertIds(this->CurrentNewFaceId);
  }
  virtual vtkIdTypeArray* GetCreatedModelEdgeVertIDs()
  {
    return this->GetCreatedModelEdgeVertIDs(this->CurrentNewFaceId);
  }
  virtual vtkIdTypeArray* GetFaceEdgeLoopIDs()
  {
    return this->GetFaceEdgeLoopIDs(this->CurrentNewFaceId);
  }
  virtual vtkIdTypeArray* GetSplitEdgeVertIds(vtkIdType newfaceid)
  {
    return this->FaceSplitInfo[newfaceid].SplitEdgeVertIds;
  }
  virtual vtkIdTypeArray* GetCreatedModelEdgeVertIDs(vtkIdType newfaceid)
  {
    return this->FaceSplitInfo[newfaceid].CreatedModelEdgeVertIDs;
  }
  virtual vtkIdTypeArray* GetFaceEdgeLoopIDs(vtkIdType newfaceid)
  {
    return this->FaceSplitInfo[newfaceid].FaceEdgeLoopIDs;
  }

protected:
  vtkSplitOperationBase();
  ~vtkSplitOperationBase() override;

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

  // Description:
  // This is particularly for caching info for splitting hybrid model faces
  // with edges so that the changes can be tranfered to client model.
  // <NewFaceId, FaceEdgeSplitInfo>
  std::map<vtkIdType, FaceEdgeSplitInfo> FaceSplitInfo;

  // Description:
  // The split angle for the operator in degrees.
  vtkIdType CurrentNewFaceId;

private:
  // Description:
  // The list of created model face ids.
  vtkIdTypeArray* CreatedModelFaceIDs;

  // Description:
  // The unique persistent id of the model entity to be operated on.
  // This is required to be set before calling Operate.
  vtkIdType Id;
  int IsIdSet;

  // Description:
  // The split angle for the operator in degrees.
  double FeatureAngle;
  int IsFeatureAngleSet;

  vtkSplitOperationBase(const vtkSplitOperationBase&); // Not implemented.
  void operator=(const vtkSplitOperationBase&);        // Not implemented.
};

#endif
