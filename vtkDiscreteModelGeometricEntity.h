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
// .NAME vtkDiscreteModelGeometricEntity - Abstract class that is used through
// multiple inheritance to imbue the required functionality for vtkDiscreteModelFace,
// vtkDiscreteModelEdge, vtkDiscreteModelVertex, and vtkDiscreteModelRegion objects.
// .SECTION Description
// Abstract class that is used through
// multiple inheritance to imbue the required functionality for vtkDiscreteModelFace,
// vtkDiscreteModelEdge, vtkDiscreteModelVertex, and vtkDiscreteModelRegion objects.  A cell data
// array is used to get the corresponding cell id for the master grid.


#ifndef __vtkDiscreteModelGeometricEntity_h
#define __vtkDiscreteModelGeometricEntity_h

#include "vtkDiscreteModelModule.h" // For export macro
#include "vtkDiscreteModelEntity.h"

#include <vector>
#include "vtkType.h"

class vtkModelMaterial;
class vtkIdList;
class vtkIdTypeArray;
class vtkModel;
class vtkModelGeometricEntity;

class VTKDISCRETEMODEL_EXPORT vtkDiscreteModelGeometricEntity : public vtkDiscreteModelEntity
{
public:

  // Description:
  // Merge the source model entity into this model entity.  lowerDimensionalIds
  // is the entity ids of model entities that are only on the boundary of
  // the source and target.  They will get destroyed during the merge operation.
  virtual bool Merge(vtkDiscreteModelGeometricEntity* source,
                     vtkIdTypeArray* lowerDimensionalIds);

  // Description:
  // Get a pointer to this object that is a vtkModelEntity.
  virtual vtkModelEntity* GetThisModelEntity()=0;

  // Description:
  // Returns the material.  Should return zero values for objects that
  // are part of the boundary of a higher dimensional object.
  virtual vtkModelMaterial* GetMaterial();

  // Description:
  // Get the CellId on the master vtkPolyData for the cell with
  // inputted Id on this vtkPolyData.  Note that this operation
  // is only valid on the server. Returns -1 if there is no
  // vtkPolyData or Id is greater than the number of cells.
  vtkIdType GetMasterCellId(vtkIdType id);

  // Description:
  // Get the number of cells for discretizing this geometric entity.
  vtkIdType GetNumberOfCells();

  // Description:
  // Return a pointer to this if it is a vtkDiscreteModelGeometricEntity given
  // a vtkModelEntity.  Currently this returns non-null values for vtkDiscreteModelFaces
  // and vtkDiscreteModelRegions.
  static vtkDiscreteModelGeometricEntity* GetThisDiscreteModelGeometricEntity(vtkModelEntity*);

  // Description:
  // Get name of the cell array that maps the CellId for this grid to the CellId on
  // the master grid.
  static const char* GetReverseClassificationArrayName();

protected:
  vtkDiscreteModelGeometricEntity();
  virtual ~vtkDiscreteModelGeometricEntity();

//BTX
  // for using AddCellsToGeometry
  friend class vtkDiscreteModel;
  friend class vtkCMBParserBase;
  friend class vtkDiscreteModelWrapper;
  friend class vtkCMBModelStateOperator;
  friend class vtkCMBModelBuilder;
  friend class vtkCmbMeshToModelWriter;
  friend class CmbGeologyBuilderCore;
  friend class CmbSceneBuilderCore;
//ETX

  // Description:
  // Add cells to this geometric representation.  This should
  // only be called from vtkDiscreteModel on the server as vtkDiscreteModel is
  // responsible for removing this cell from the current
  // vtkDiscreteModelGeometricEntity that is classified on.
  virtual bool AddCellsToGeometry(vtkIdList* cellIds);

  // This can be overriden by subclasses to modify the behavior of
  // adding the cell ids classification to the mesh
  virtual bool AddCellsClassificationToMesh(vtkIdList* cellIds);

  // Description:
  // Get the array that maps the CellId for this grid to the CellId on
  // the master grid.
  vtkIdTypeArray* GetReverseClassificationArray();

  void SetMaterial(vtkModelMaterial* material);
//BTX
  friend class vtkModelMaterial;
//ETX

private:
  vtkDiscreteModelGeometricEntity(const vtkDiscreteModelGeometricEntity&);  // Not implemented.
  void operator=(const vtkDiscreteModelGeometricEntity&);  // Not implemented.

  // Description:
  // Remove a list of cells from the geometric object.  The cell
  // Ids are with respect to the geometric entity, not with respect
  // to the master geometry.  This is a private function as the only
  // function that I can see that would need to call this is AddCellsToGeometry.
  bool RemoveCellsFromGeometry(vtkIdList* cellIds);
};

#endif

