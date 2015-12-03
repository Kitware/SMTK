//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkDiscreteModelEdge - Abstract generic model entity class.
// .SECTION Description

#ifndef __smtkdiscrete_vtkDiscreteModelEdge_h
#define __smtkdiscrete_vtkDiscreteModelEdge_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "Model/vtkModelEdge.h"
#include "vtkDiscreteModelGeometricEntity.h"


class vtkModelEdgeUse;
class vtkModelItemIterator;
class vtkModelVertex;
class vtkModelRegion;
class vtkIdList;

class VTKSMTKDISCRETEMODEL_EXPORT vtkDiscreteModelEdge : public vtkModelEdge,
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

  // Description:
  // Check if the pointId is used by this model edge cells.
  virtual bool IsEdgeCellPoint(vtkIdType pointId);

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
  friend class vtkDiscreteModelFace;
  friend class vtkDiscreteModelWrapper;
  friend class vtkCMBMapToCMBModel;
  friend class vtkCMBModelStateOperator;
  friend class vtkCMBModelBuilder;
  friend class vtkCMBMeshToModelWriter;
  friend class pqCMBGeologyBuilderMainWindowCore;
  friend class pqCMBSceneBuilderMainWindowCore;
  friend class vtkCreateModelEdgesOperator;
  //ETX

  // Description:
  // Add cells to this geometric representation.  This should
  // only be called from vtkDiscreteModel on the server as vtkDiscreteModel is
  // responsible for removing this cell from the current
  // vtkDiscreteModelGeometricEntity that is classified on.
  // Note that it can potentially modify the values inside
  //  of cellIds.
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

