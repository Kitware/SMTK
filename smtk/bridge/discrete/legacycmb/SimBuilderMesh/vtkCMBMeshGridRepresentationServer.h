//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBMeshGridRepresentationServer - CMBModel representation of an analysis grid from a BC file.
// .SECTION Description
// A class used to provide all of the information that a CMBModel needs
// to keep track of mapping grid objects from the geometry grid to the
// analysis grid.  The source of this information is the Mesh generated in SimBuilder
// This class assumes that the analysis grid and the
// model grid share the same boundary grid points and a direct
// correspondence between boundary cells and analysis cell boundaries.
// It also assumes that the boundary mesh is made up of triangles
// The values are 0-based indexed.  Note also that
// we duplicately store cell and cell side information for cells
// that are adjacent to an internal model face (i.e. a model
// face that is adjacent to 2 model regions).

#ifndef __vtkCMBMeshGridRepresentationServer_h
#define __vtkCMBMeshGridRepresentationServer_h

#include "cmbSystemConfig.h"
#include "vtkModelGeneratedGridRepresentation.h"
#include "vtkWeakPointer.h"
#include <map>
#include <set>
#include <vector>

class vtkPolyData;
class vtkCMBMeshServer;
class vtkDiscreteModel;
class vtkAlgorithm;
class vtkIntArray;
class vtkModelEntity;
class vtkIdTypeArray;

class VTK_EXPORT vtkCMBMeshGridRepresentationServer : public vtkModelGeneratedGridRepresentation
{
public:
  static vtkCMBMeshGridRepresentationServer* New();
  vtkTypeMacro(vtkCMBMeshGridRepresentationServer, vtkModelGeneratedGridRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // See vtkModelGridRepresentation.
  virtual bool GetBCSNodalAnalysisGridPointIds(
    vtkDiscreteModel* model, vtkIdType bcsGroupId, int bcGroupType, vtkIdList* pointIds);

  // Description:
  // See vtkModelGridRepresentation.
  virtual bool GetFloatingEdgeAnalysisGridPointIds(
    vtkDiscreteModel* model, vtkIdType modelEdgeId, vtkIdList* pointIds);

  // Description:
  // See vtkModelGridRepresentation.
  virtual bool GetModelEdgeAnalysisPoints(
    vtkDiscreteModel* model, vtkIdType edgeId, vtkIdTypeArray* edgePoints);

  // Description:
  // See vtkModelGridRepresentation.
  virtual bool GetBoundaryGroupAnalysisFacets(
    vtkDiscreteModel* model, vtkIdType boundaryGroupId, vtkIdList* cellIds, vtkIdList* cellSides);

  // Description:
  // Do some type of validation of the mapping information in model.
  // So far we can't guarantee that this works.
  virtual bool IsModelConsistent(vtkDiscreteModel* model);

  // Description:
  // Initialize the information from a sim mesh or mesh representation.
  // Returns true for success.
  bool Initialize(vtkCMBMeshServer* mesh);
  bool Initialize(vtkPolyData* meshRepresentation, vtkDiscreteModel* model);

  // Description:
  // clear the analysis grid info.
  virtual void Reset();

  // Description:
  // The method to set the mesh representation input. This is
  // useful if some filters are applied to the mesh, and the representation
  // is changed due to that.
  vtkGetObjectMacro(Representation, vtkPolyData);
  void SetRepresentation(vtkPolyData* mesh);

  // Description:
  // Get cellIds or area given a group id of model entities.
  // Meant for 2D models with triangle meshes.
  virtual bool GetGroupFacetIds(vtkDiscreteModel* model, int groupId, std::vector<int>& cellIds);
  virtual bool GetGroupFacetsArea(vtkDiscreteModel* model, int groupId, double& area);

  // Description:
  // Get cell and point information from the analysis grid.
  virtual bool GetCellPointIds(int cellId, std::vector<int>& pointIds);
  virtual bool GetPointLocation(int pointId, std::vector<double>& coords);

  virtual void WriteMeshToFile();

protected:
  vtkCMBMeshGridRepresentationServer();
  virtual ~vtkCMBMeshGridRepresentationServer();

  friend class vtkCMBMeshToModelWriter;
  bool RepresentationBuilt;
  bool BuildRepresentation(vtkCMBMeshServer* meshServer);
  vtkIdTypeArray* GetCellIdMapArray();
  vtkIntArray* GetCellTypeMapArray();
  vtkIdTypeArray* GetPointIdMapArray();
  vtkIntArray* GetPointTypeMapArray();
  vtkIdTypeArray* GetCellPointIdsArray();
  bool CanProcessModelGroup(vtkDiscreteModel* model, int groupId, std::set<vtkIdType>& faceIdList);

  vtkPolyData* Representation;
  vtkWeakPointer<vtkDiscreteModel> Model;

private:
  vtkCMBMeshGridRepresentationServer(const vtkCMBMeshGridRepresentationServer&); // Not implemented.
  void operator=(const vtkCMBMeshGridRepresentationServer&);                     // Not implemented.
};
#endif
