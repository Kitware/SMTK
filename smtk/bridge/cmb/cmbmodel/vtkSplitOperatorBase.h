/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME vtkSplitOperatorBase - Split a model face
// .SECTION Description
// Operator to split a model face given an angle.

#ifndef __smtkcmb_vtkSplitOperatorBase_h
#define __smtkcmb_vtkSplitOperatorBase_h

#include "SMTKCMBBridgeExports.h" // For export macro
#include "vtkObject.h"
#include "ModelEdgeHelper.h"


class vtkDiscreteModel;
class vtkIdTypeArray;
class vtkModelEntity;

class SMTKCMBBRIDGE_EXPORT vtkSplitOperatorBase : public vtkObject
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
