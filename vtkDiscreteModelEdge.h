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
// .NAME vtkDiscreteModelEdge - Abstract generic model entity class.
// .SECTION Description

#ifndef __vtkDiscreteModelEdge_h
#define __vtkDiscreteModelEdge_h

#include "vtkDiscreteModelModule.h" // For export macro
#include "Model/vtkModelEdge.h"
#include "vtkDiscreteModelGeometricEntity.h"

class vtkModelEdgeUse;
class vtkModelItemIterator;
class vtkModelVertex;
class vtkModelRegion;
class vtkIdList;

class VTKDISCRETEMODEL_EXPORT vtkDiscreteModelEdge : public vtkModelEdge,
  public vtkDiscreteModelGeometricEntity
{
public:
  vtkTypeMacro(vtkDiscreteModelEdge,vtkModelEdge);
  static vtkDiscreteModelEdge* New();
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Add the association to a region.  This is intended for
  // "floating" edges used to model things like wells that are
  // not adjacent to a model face.
  void AddRegionAssociation(vtkIdType regionId);

  // Description:
  // Return the vtkModelRegion that contains this floating edge.
  // This is intended for "floating" edges used to model things
  // like wells that are not adjacent to a model face.
  vtkModelRegion* GetModelRegion();

  // Description:
  // Get/Set the line resolution of the edge.
  static vtkInformationIntegerKey* LINERESOLUTION();
  void SetLineResolution(int resolution);
  int GetLineResolution();

  // Function for settting/getting the information object
  // used to store the geometric representation of the edge with points
  static vtkInformationObjectBaseKey* LINEADNPOINTSGEOMETRY();
  void SetLineAndPointsGeometry(vtkObject* geometry);
  vtkObject* GetLineAndPointsGeometry();

  // Description:
  // Description:
  // Split this model edge based on a point id in the grid.  Note
  // that the pointId is independent of the grid as all grids
  // use all the same points.  The function fills the created
  // model vertex and edge UniquePersistentIds in createdVertexId
  // and createdEdgeId respectively, and returns true if successful.
  bool Split(vtkIdType pointId, vtkIdType & createdVertexId,
             vtkIdType & createdEdgeId);

  // Definition:
  // Get the representation for the geometry. On the client
  // this will be a vtkSMProxy* and on the server it will be
  // a vtkDataObject*.  If we are on the server and this is a
  // floating edge then we will construct the representation
  // if it doesn't exist.
  virtual vtkObject* GetGeometry();

  // Description:
  // Get All/Boundary/Interior point Ids of this model edge.
  virtual void GetAllPointIds(vtkIdList* ptsList);
  virtual void GetInteriorPointIds(vtkIdList* ptsList);
  virtual void GetBoundaryPointIds(vtkIdList* ptsList);

protected:
  vtkDiscreteModelEdge();
  virtual ~vtkDiscreteModelEdge();

  virtual bool IsDestroyable();
  virtual bool Destroy();

  virtual vtkModelEntity* GetThisModelEntity();

  //BTX
  // for using AddCellsToGeometry
  friend class vtkDiscreteModel;
  friend class vtkCMBParserBase;
  friend class vtkDiscreteModelWrapper;
  friend class vtkCmbMapToCmbModel;
  friend class vtkCMBModelStateOperator;
  friend class vtkCMBModelBuilder;
  friend class vtkCmbMeshToModelWriter;
  friend class CmbGeologyBuilderCore;
  friend class CmbSceneBuilderCore;
  friend class vtkCreateModelEdgesOperator;
  //ETX

  // Description:
  // Add cells to this geometric representation.  This should
  // only be called from vtkDiscreteModel on the server as vtkDiscreteModel is
  // responsible for removing this cell from the current
  // vtkDiscreteModelGeometricEntity that is classified on.
  virtual bool AddCellsToGeometry(vtkIdList* cellIds);

  virtual bool AddCellsClassificationToMesh(vtkIdList* cellIds);

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

  // Description:
  // This function is intended for use with floating model edges.
  // Construct the vtkPolyData representation from the two end points
  // and the line resolution. Returns true if successful or
  // if the model edge already contains a representation.
  bool ConstructRepresentation();

  friend class vtkEdgeSplitOperatorClient;
  // Description:
  // Function to split a model edge if it is a loop (i.e. has
  // no beginning or end point)
  bool SplitModelEdgeLoop(vtkIdType pointId);

private:
  vtkDiscreteModelEdge(const vtkDiscreteModelEdge&);  // Not implemented.
  void operator=(const vtkDiscreteModelEdge&);  // Not implemented.
};

#endif

