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
// .NAME vtkMergeOperatorBase - Merge two geometric model entities
// .SECTION Description
// Operator to merge a source geometric model entity into
// a target geometric entity.  The properties of the target entity
// (e.g. color, BCS/ModelEntityGroup associations) will not be changed.


#ifndef __smtkcmb_vtkMergeOperatorBase_h
#define __smtkcmb_vtkMergeOperatorBase_h

#include "smtk/bridge/cmb/cmbBridgeExports.h" // For export macro
#include "vtkObject.h"


class vtkDiscreteModel;
class vtkDiscreteModelGeometricEntity;
class vtkIdTypeArray;

class SMTKCMBBRIDGE_EXPORT vtkMergeOperatorBase : public vtkObject
{
public:
  static vtkMergeOperatorBase * New();
  vtkTypeMacro(vtkMergeOperatorBase,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get the target vtkDiscreteModelGeometricEntity's unique persistent
  // id to operate on. This is required to be set before calling Operate.
  vtkGetMacro(IsTargetIdSet, int);
  vtkGetMacro(TargetId, vtkIdType);
  void SetTargetId(vtkIdType targetId);

//BTX
  // Description:
  // Get the vtkDiscreteModelGeometricEntity corresponding to TargetId.
  vtkDiscreteModelGeometricEntity* GetTargetModelEntity(vtkDiscreteModel* model);

  // Description:
  // Get the vtkDiscreteModelGeometricEntity corresponding to SourceId.
  vtkDiscreteModelGeometricEntity* GetSourceModelEntity(vtkDiscreteModel* model);
//ETX

  // Description:
  // Set/get the source vtkDiscreteModelGeometricEntity's unique persistent
  // id to operate on. This is required to be set before calling Operate.
  vtkGetMacro(IsSourceIdSet, int);
  vtkGetMacro(SourceId, vtkIdType);
  void SetSourceId(vtkIdType sourceId);

  // Description:
  // Get/set the list of vtkDiscreteModelGeometricEntity Ids that are
  // to be deleted.  Note that they must be one dimensional lower
  // than the source and target model entities.
  vtkGetObjectMacro(LowerDimensionalIds, vtkIdTypeArray);
  void SetLowerDimensionalIds(vtkIdTypeArray*);

  // Description:
  // Add another lower dimensional id to LowerDimensionalIds.
  void AddLowerDimensionalId(vtkIdType id);

  // Description:
  // Remove all of the ids in LowerDimensionalIds.
  void RemoveAllLowerDimensionalIds();

//BTX
  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);
//ETX
protected:
  vtkMergeOperatorBase();
  virtual ~vtkMergeOperatorBase();

  // Description:
  // Do the basic merge operation that needs to get done on both the
  // client and the server and return true if it succeeded.
  virtual bool Operate(vtkDiscreteModel* Model);

private:
  // Description:
  // The vtkDiscreteModelGeometricEntity Id to be merged into
  // the target.  Note that the source must be of the same type
  // as the target.
  vtkIdType SourceId;
  int IsSourceIdSet;

  // Description:
  // A list of one dimensional lower geometric objects than the source and target
  // model entities that are shared between the source and target
  // entities that are also to be deleted.  This is useful for the
  // cases where a source and target share more than a single
  // lower dimensional model entity.
  vtkIdTypeArray* LowerDimensionalIds;

  // Description:
  // The unique persistent id of the vtkDiscreteModelGeometricEntity
  // that is the target of the merge.
  // This is required to be set before calling Operate.
  vtkIdType TargetId;
  int IsTargetIdSet;

  vtkMergeOperatorBase(const vtkMergeOperatorBase&);  // Not implemented.
  void operator=(const vtkMergeOperatorBase&);  // Not implemented.
};

#endif
