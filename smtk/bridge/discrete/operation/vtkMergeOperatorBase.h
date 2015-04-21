//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkMergeOperatorBase - Merge two geometric model entities
// .SECTION Description
// Operator to merge a source geometric model entity into
// a target geometric entity.  The properties of the target entity
// (e.g. color, BCS/ModelEntityGroup associations) will not be changed.


#ifndef __smtkdiscrete_vtkMergeOperatorBase_h
#define __smtkdiscrete_vtkMergeOperatorBase_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkObject.h"


class vtkDiscreteModel;
class vtkDiscreteModelGeometricEntity;
class vtkIdTypeArray;

class SMTKDISCRETESESSION_EXPORT vtkMergeOperatorBase : public vtkObject
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
