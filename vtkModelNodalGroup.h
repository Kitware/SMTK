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
// .NAME vtkModelNodalGroup - An object to store a set of point Ids.
// .SECTION Description
// An object that stores a group of point Ids.  All point Ids are
// stored uniquely.

#ifndef __vtkModelNodalGroup_h
#define __vtkModelNodalGroup_h

#include "vtkModelEntity.h"
#include <set>

class vtkBitArray;
class vtkDiscreteModel;
class vtkDiscreteModelFace;
class vtkDiscreteModelEdge;
class vtkModelEntity;
struct vtkModelNodalGroupInternals;
class vtkPolyData;

typedef enum {
  BASE_NODAL_GROUP = 0,
  UNIQUE_NODAL_GROUP = 1
} VTKModelNodalGroupType;

//BTX
typedef enum
{
  vtkDiscreteModelEntityAllPoints = 0,
  vtkDiscreteModelEntityBoundaryPoints,
  vtkDiscreteModelEntityInteriorPoints
} enNodalGroupPointLocationType;
//ETX

class VTK_EXPORT vtkModelNodalGroup : public vtkModelEntity
{
public:
  vtkTypeMacro(vtkModelNodalGroup,vtkModelEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Add/remove/clear model entity from the set.
  virtual void AddModelEntity(vtkModelEntity* ModelEntity);
  virtual void RemoveModelEntity(vtkModelEntity* ModelEntity);
  virtual void ClearEntities();
  std::set<vtkModelEntity*>& GetModelEntities() const;

  // Description:
  // Set/get the PointLocationType.
  //typedef enum
  //  {
  //  vtkDiscreteModelEntityAllPoints = 0,
  //  vtkDiscreteModelEntityBoundaryPoints,
  //  vtkDiscreteModelEntityInteriorPoints
  //  } enNodalGroupPointLocationType;
  vtkGetMacro(PointLocationType, int);
  vtkSetMacro(PointLocationType, int);

  // Description:
  // Clear the list of point Ids.
  void ClearPointIds();

  // Description:
  // Returns the number of model entities of type this->EntityType
  // that is grouped by this object.
  vtkIdType GetNumberOfPointIds();

  // Description:
  // Get a list of point Ids.
  void GetPointIds(vtkIdList* PointIds);

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

  // Description:
  // Return the type.
  virtual int GetType();

  // Description:
  // Construct the vtkPolyData representation from the set of points
  // in the group.  Returns true if successful.
  bool ConstructRepresentation(vtkPolyData* Grid);

  // Description:
  // Get the model that this object is associated with.
  vtkDiscreteModel* GetModel();

  // Description:
  // Flag to indicate the type of vtkModelNodalGroup.
  virtual int GetNodalGroupType() {return BASE_NODAL_GROUP;}

protected:
  vtkModelNodalGroup();
  virtual ~vtkModelNodalGroup();
  static vtkModelNodalGroup *New();
//BTX
  friend class vtkDiscreteModel;
//ETX

  // Description:
  // Add/remove point Ids to/from the set.
  virtual void AddPointId(vtkIdType PointId);
  virtual void RemovePointId(vtkIdType PointId);
  virtual void AddPointIds(vtkIdList* PointIds);
  virtual void RemovePointIds(vtkIdList* PointIds);
  virtual void AddPointIdsInModelFace(vtkDiscreteModelFace* ModelFace);
  virtual void AddPointIdsInModelEdge(vtkDiscreteModelEdge* ModelEdge);
  virtual void AddPointIdsInModelFaceInterior(vtkDiscreteModelFace* ModelFace);
  virtual void AddPointIdsInModelFaceBoundary(vtkDiscreteModelFace* ModelFace);
  virtual void RemovePointIdsInModelFace(vtkDiscreteModelFace* ModelFace);
  virtual void RemovePointIdsInModelFaceInterior(vtkDiscreteModelFace* ModelFace);
  virtual void RemovePointIdsInModelFaceBoundary(vtkDiscreteModelFace* ModelFace);

  // Description:
  // Mark each index in  Points with 0 (out) or 1 (in) for vtkPoints that are
  // in the vtkDiscreteModelEdge grid.
  void GatherAllPointIdsOfModelEdge(vtkDiscreteModelEdge* ModelEdge,
    vtkBitArray* Points);

  virtual bool IsDestroyable();
  virtual bool Destroy();

  // Description:
  // Contains std::set<vtkIdType> PointIds with respect to the
  // master grid.
  vtkModelNodalGroupInternals* Internal;

  // Description:
  // A flag to indicate what points in the model entity will
  // be used for the nodal group.
  //typedef enum
  //  {
  //  vtkDiscreteModelEntityAllPoints = 0,
  //  vtkDiscreteModelEntityBoundaryPoints,
  //  vtkDiscreteModelEntityInteriorPoints
  //  } enNodalGroupPointLocationType;
  int PointLocationType;

private:
  vtkModelNodalGroup(const vtkModelNodalGroup&);  // Not implemented.
  void operator=(const vtkModelNodalGroup&);  // Not implemented.
};

#endif

