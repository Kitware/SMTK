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
// .NAME vtkSelectionSplitOperatorBase - Split a model face based on a vtkSelection.
// .SECTION Description
// Operator to split model faces based on a vtkSelection. Note
// that the cell Ids included in the vtkSelection are with respect
// to the master vtkPolyData.

#ifndef __vtkSelectionSplitOperatorBase_h
#define __vtkSelectionSplitOperatorBase_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include "ModelEdgeHelper.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkIdTypeArray;

class VTKCMBDISCRETEMODEL_EXPORT vtkSelectionSplitOperatorBase : public vtkObject
{
public:
  static vtkSelectionSplitOperatorBase * New();
  vtkTypeMacro(vtkSelectionSplitOperatorBase,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the list of modified geometric model entity pairs during a
  // split operation.  If you are accessing the information inside
  // the array you should strongly consider using AddModifiedPair()
  // or GetModifiedPair() instead.
  vtkGetObjectMacro(ModifiedPairIDs, vtkIdTypeArray);

  // Description:
  // Get the list of completely selected model faces during a split operation.
  vtkGetObjectMacro(CompletelySelectedIDs, vtkIdTypeArray);

  // Description:
  // Add a pair of IDs of modified geometric model entities.  SourceID is
  // the Id of the geometric model entity that was split and TargetID is
  // the Id of the geometric model entity that was created from the split.
  void AddModifiedPair(vtkIdType SourceID, vtkIdType TargetID);

  // Description:
  // Get a pair of IDs of modified geometric model entities.  SourceID is
  // the Id of the geometric model entity that was split and TargetID is
  // the Id of the geometric model entity that was created from the split.
  // The function returns true if it succeeded.
  bool GetModifiedPair(vtkIdType index, vtkIdType & SourceID,
                       vtkIdType & TargetID);

  // Description:
  // Set/get the current existing face Id, which is used to retrieve
  // the cached SplitInfo
  vtkGetMacro(CurrentExistingFaceId, vtkIdType);
  vtkSetMacro(CurrentExistingFaceId, vtkIdType);

  // Description:
  // Get FaceSplitInfo arrays AFTER a split operation, based on
  // CurrentExistingFaceId
  virtual vtkIdTypeArray* GetSplitEdgeVertIds()
  { return this->GetSplitEdgeVertIds(this->CurrentExistingFaceId);}
  virtual vtkIdTypeArray* GetCreatedModelEdgeVertIDs()
  { return this->GetCreatedModelEdgeVertIDs(this->CurrentExistingFaceId);}
  virtual vtkIdTypeArray* GetFaceEdgeLoopIDs()
  { return this->GetFaceEdgeLoopIDs(this->CurrentExistingFaceId);}
  virtual vtkIdTypeArray* GetSplitEdgeVertIds(vtkIdType existingfaceid)
  { return this->FaceSplitInfo[existingfaceid].SplitEdgeVertIds;}
  virtual vtkIdTypeArray* GetCreatedModelEdgeVertIDs(vtkIdType existingfaceid)
  { return this->FaceSplitInfo[existingfaceid].CreatedModelEdgeVertIDs;}
  virtual vtkIdTypeArray* GetFaceEdgeLoopIDs(vtkIdType existingfaceid)
  { return this->FaceSplitInfo[existingfaceid].FaceEdgeLoopIDs;}

protected:
  vtkSelectionSplitOperatorBase();
  virtual ~vtkSelectionSplitOperatorBase();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

  // Description:
  // This is particularly for caching info for splitting hybrid model faces
  // with edges so that the changes can be tranfered to client model.
  // <ExistingFaceId, FaceEdgeSplitInfo>
  std::map<vtkIdType, FaceEdgeSplitInfo> FaceSplitInfo;

  // Description:
  // The split angle for the operator in degrees.
  vtkIdType CurrentExistingFaceId;

private:
  // Description:
  // The list of modified geometric model entities.  The number of components
  // in the array is two with the first tuple Id corresponding to the
  // geometric model entity that had cells removed from it (the source)
  // and the second Id corresponding to the geometric model entity
  // that was created from the source.
  vtkIdTypeArray* ModifiedPairIDs;

  // Description:
  // The list of model faces that were completely selected and thus
  // not split.  This may be useful if a BCS/Entity Group is to be applied
  // to all of the resulting model faces.
  vtkIdTypeArray* CompletelySelectedIDs;

  vtkSelectionSplitOperatorBase(const vtkSelectionSplitOperatorBase&);  // Not implemented.
  void operator=(const vtkSelectionSplitOperatorBase&);  // Not implemented.
};

#endif
