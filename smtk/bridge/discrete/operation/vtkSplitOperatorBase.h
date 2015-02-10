//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkSplitOperatorBase - Split a model face
// .SECTION Description
// Operator to split a model face given an angle.

#ifndef __smtkdiscrete_vtkSplitOperatorBase_h
#define __smtkdiscrete_vtkSplitOperatorBase_h

#include "smtk/bridge/discrete/discreteSessionExports.h" // For export macro
#include "vtkObject.h"
#include "ModelEdgeHelper.h"


class vtkDiscreteModel;
class vtkIdTypeArray;
class vtkModelEntity;

class SMTKDISCRETESESSION_EXPORT vtkSplitOperatorBase : public vtkObject
{
public:
  static vtkSplitOperatorBase * New();
  vtkTypeMacro(vtkSplitOperatorBase,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

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

//BTX
  // Description:
  // Return the model entity.
  vtkModelEntity* GetModelEntity(vtkDiscreteModel* Model);
//ETX

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
  { return this->GetSplitEdgeVertIds(this->CurrentNewFaceId);}
  virtual vtkIdTypeArray* GetCreatedModelEdgeVertIDs()
  { return this->GetCreatedModelEdgeVertIDs(this->CurrentNewFaceId);}
  virtual vtkIdTypeArray* GetFaceEdgeLoopIDs()
  { return this->GetFaceEdgeLoopIDs(this->CurrentNewFaceId);}
  virtual vtkIdTypeArray* GetSplitEdgeVertIds(vtkIdType newfaceid)
  { return this->FaceSplitInfo[newfaceid].SplitEdgeVertIds;}
  virtual vtkIdTypeArray* GetCreatedModelEdgeVertIDs(vtkIdType newfaceid)
  { return this->FaceSplitInfo[newfaceid].CreatedModelEdgeVertIDs;}
  virtual vtkIdTypeArray* GetFaceEdgeLoopIDs(vtkIdType newfaceid)
  { return this->FaceSplitInfo[newfaceid].FaceEdgeLoopIDs;}

protected:
  vtkSplitOperatorBase();
  virtual ~vtkSplitOperatorBase();

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

  vtkSplitOperatorBase(const vtkSplitOperatorBase&);  // Not implemented.
  void operator=(const vtkSplitOperatorBase&);  // Not implemented.
};

#endif
