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
// .NAME vtkCmbMeshGridRepresentation - CMBModel representation of an analysis grid from a BC file.
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

#ifndef __vtkCmbMeshGridRepresentation_h
#define __vtkCmbMeshGridRepresentation_h

#include "vtkCmbGridRepresentation.h"
#include <map>
#include <set>

class VTK_EXPORT vtkCmbMeshGridRepresentation : public vtkCmbGridRepresentation
{
public:
  static vtkCmbMeshGridRepresentation* New();
  vtkTypeRevisionMacro(vtkCmbMeshGridRepresentation,vtkCmbGridRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // See vtkCmbGridRepresentation.
  virtual bool GetNodalGroupAnalysisGridPointIds(vtkCMBModel* model, vtkIdType nodalGroupId,
                                                 vtkIdList* pointIds);

  // Description:
  // See vtkCmbGridRepresentation.
  virtual bool GetFloatingEdgeAnalysisGridPointIds(vtkCMBModel* model, vtkIdType modelEdgeId,
                                                   vtkIdList* pointIds);

  // Description:
  // See vtkCmbGridRepresentation.
  virtual bool GetModelEdgeAnalysisPoints(vtkCMBModel* model, vtkIdType edgeId,
                                          vtkIdTypeArray* edgePoints);

  // Description:
  // See vtkCmbGridRepresentation.
  virtual bool GetBoundaryGroupAnalysisFacets(vtkCMBModel* model, vtkIdType boundaryGroupId,
                                              vtkIdList* cellIds, vtkIdList* cellSides);

  // Description:
  // Do some type of validation of the mapping information in model.
  // So far we can't guarantee that this works.
  virtual bool IsModelConsistent(vtkCMBModel* model);

  // Description:
  // Initialize the information from a sim mesh.
  // Returns true for success.
  bool Initialize(vtkCMBModel* model);

  bool AddNodalGroup(vtkIdType nodalGroupId, vtkIdList* pointIds, vtkCMBModel* model);

  bool AddFloatingEdge(vtkIdType floatingEdgeId, vtkIdList* pointIds, vtkCMBModel* model);

  bool AddModelFace(vtkIdType modelFaceId, vtkIdList* cellIds, vtkIdList* cellSides, vtkCMBModel* model);

  // Description:
  // Set GridFileName to NULL and clear the analysis grid info.
  virtual void Reset();

  // Description:
  // Check to see if a floating edge has the same amount of points in it as
  // the corresponding set in FloatingEdgeToPointIds.  Returns true if it is.
  bool IsFloatingEdgeConsistent(vtkCMBModel* model, vtkIdType floatingEdgeId);

  // Description:
  // Check to see if a nodal group has the same amount of points in it as
  // the corresponding set in NodalGroupToPointIds.  Returns true if it is.
  bool IsNodalGroupConsistent(vtkCMBModel* model, vtkIdType nodalGroupId);

  // Description:
  // Check to see if a boundary group has the same amount of facets in it as
  // the corresponding set in BoundaryGroupFacetInfo.  Returns true if it is.
  bool IsModelFaceConsistent(vtkCMBModel* model, vtkIdType boundaryGroupId);

  // Description:
  // Get the analysis grid information for a given model face.
  virtual bool GetModelFaceAnalysisFacets(vtkCMBModel* model, vtkIdType modelFaceId,
                                          vtkIdList* cellIds, vtkIdList* cellSides);

protected:
  vtkCmbMeshGridRepresentation();
  virtual ~vtkCmbMeshGridRepresentation();

private:
  vtkCmbMeshGridRepresentation(const vtkCmbMeshGridRepresentation&);  // Not implemented.
  void operator=(const vtkCmbMeshGridRepresentation&);  // Not implemented.

  // Description:
  // A mapping from a nodal group id to a set of point ids in the analysis grid.
  std::map<vtkIdType, std::set<vtkIdType> > NodalGroupToPointIds;

  // Description:
  // A mapping from a model edge id to a set of point ids in the analysis grid.
  // It's assumed that we have a 3D problem and the edges are all "floating".
  std::map<vtkIdType, std::set<vtkIdType> > FloatingEdgeToPointIds;

  // Description:
  // A mapping from a boundary group id to a set of analysis grid cells and sides.
  // For now we ignore the point ids of the cell sides even though it's in the bc file.
  std::map<vtkIdType, std::set<std::pair<vtkIdType, int> > > ModelFaceToFacetInfo;
};
#endif
