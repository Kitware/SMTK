//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelBCGridRepresentation - CMBModel representation of an analysis grid from a BC file.
// .SECTION Description
// A class used to provide all of the information that a CMBModel needs
// to keep track of mapping grid objects from the geometry grid to the
// analysis grid.  The source of this information is the BC file returned from Omicron.
// This class assumes that the analysis grid and the
// model grid share the same boundary grid points and a direct
// correspondence between boundary cells and analysis cell boundaries.
// It also assumes that the boundary mesh is either lines, tris, or quads.
// The values are stored in C/C++/0-based indexing.  Note also that
// we duplicately store cell and cell side information for cells
// that are adjacent to an internal model face (i.e. a model
// face that is adjacent to 2 model regions).

#ifndef __smtkdiscrete_vtkModelBCGridRepresentation_h
#define __smtkdiscrete_vtkModelBCGridRepresentation_h

#include "smtk/session/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelGridRepresentation.h"
#include "vtkSmartPointer.h"

#include <map>
#include <set>

class vtkPolyData;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelBCGridRepresentation : public vtkModelGridRepresentation
{
public:
  static vtkModelBCGridRepresentation* New();
  vtkTypeMacro(vtkModelBCGridRepresentation, vtkModelGridRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // See vtkModelGridRepresentation.
  bool GetBCSNodalAnalysisGridPointIds(
    vtkDiscreteModel* model, vtkIdType bcsGroupId, int bcGroupType, vtkIdList* pointIds) override;

  // Description:
  // See vtkModelGridRepresentation.
  bool GetFloatingEdgeAnalysisGridPointIds(
    vtkDiscreteModel* model, vtkIdType modelEdgeId, vtkIdList* pointIds) override;

  // Description:
  // See vtkModelGridRepresentation.
  bool GetModelEdgeAnalysisPoints(
    vtkDiscreteModel* model, vtkIdType edgeId, vtkIdTypeArray* edgePoints) override;

  // Description:
  // See vtkModelGridRepresentation.
  bool GetBoundaryGroupAnalysisFacets(vtkDiscreteModel* model, vtkIdType boundaryGroupId,
    vtkIdList* cellIds, vtkIdList* cellSides) override;

  // Description:
  // Do some type of validation of the mapping information in model.
  // So far we can't guarantee that this works.
  bool IsModelConsistent(vtkDiscreteModel* model) override;

  // Description:
  // Initialize the information from a given bc file.  The format of the bc file
  // is assumed as nodes "NDS <analysis grid point Id> <nodal group id>" and
  // boundary facets "FCS <analysis grid cell Id> <cell side(1-4)>
  // <model face Id> <analysis grid cell face point Ids>".
  // Returns true for success.
  bool Initialize(const char* bcFileName, vtkDiscreteModel* model);

  bool AddFloatingEdge(vtkIdType floatingEdgeId, vtkIdList* pointIds, vtkDiscreteModel* model);

  bool AddModelFace(
    vtkIdType modelFaceId, vtkIdList* cellIds, vtkIdList* cellSides, vtkDiscreteModel* model);

  // Description:
  // Set GridFileName to NULL and clear the analysis grid info.
  void Reset() override;

  // Description:
  // Check to see if a floating edge has the same amount of points in it as
  // the corresponding set in FloatingEdgeToPointIds.  Returns true if it is.
  bool IsFloatingEdgeConsistent(vtkDiscreteModel* model, vtkIdType floatingEdgeId);

  // Description:
  // Check to see if a boundary group has the same amount of facets in it as
  // the corresponding set in BoundaryGroupFacetInfo.  Returns true if it is.
  bool IsModelFaceConsistent(vtkDiscreteModel* model, vtkIdType boundaryGroupId);

  // Description:
  // Get the analysis grid information for a given model face.
  virtual bool GetModelFaceAnalysisFacets(
    vtkDiscreteModel* model, vtkIdType modelFaceId, vtkIdList* cellIds, vtkIdList* cellSides);

protected:
  vtkModelBCGridRepresentation();
  ~vtkModelBCGridRepresentation() override;

private:
  vtkModelBCGridRepresentation(const vtkModelBCGridRepresentation&); // Not implemented.
  void operator=(const vtkModelBCGridRepresentation&);               // Not implemented.

  // Description:
  // A mapping from a model edge id to a set of point ids in the analysis grid.
  // It's assumed that we have a 3D problem and the edges are all "floating".
  std::map<vtkIdType, std::set<vtkIdType> > FloatingEdgeToPointIds;

  // Description:
  // A mapping from a master(surface) cell id to a set of analysis grid cells and sides.
  // For now we ignore the point ids of the cell sides even though it's in the bc file.
  // We are assuming, without refinement (we can't handle refined analysis mesh yet),
  // all (N) master(surface) point ids are the first N points in analysis grid.
  std::map<vtkIdType, std::set<std::pair<vtkIdType, int> > > MasterCellToMeshCellInfo;
};
#endif
