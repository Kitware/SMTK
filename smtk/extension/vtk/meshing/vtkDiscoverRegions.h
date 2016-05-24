//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_vtk_vtkDiscoverRegions_h
#define __smtk_vtk_vtkDiscoverRegions_h

#include "smtk/extension/vtk/meshing/vtkSMTKMeshingExtModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

//#define VTK_CELL_REGION_IDS "CellRegionIds"
#define VTK_CELL_REGION_IDS "Region"

namespace smtk {
  namespace vtk {

/**\brief Discover shells and regions.
 *
 * Given a triangulated surface (no strips accepted),
 * returns a set of shells composed of those facets that form closed regions
 * (plus one representing the region outside the surface).
 *
 * This filter outputs a copy of the input polydata with the
 * following additional arrays
 * <ul>
 * <li> a three-component field-data-array named "ModelFaceRegionsMap",
 *      specifying a model facet ID (component 0), a region ID for the positive
 *      facet orientation (component 2) and negative facet orientation (component 1).
 * <li> a two-component cell-data-array named "CellRegionIds",
 *      specifying the region to either side of the cell. Component 0 is
 *      the region on the back face (away from the normal). Component 1
 *      is the region on the front face (with the normal).
 * <li> a two-component field-data-array named "IsRegionAHole".
 *      Each row corresponds to a model face (just like "ModelFaceRegionsMap")
 *      and will have a non-zero entry when the region to a model face has been
 *      marked as a *volumetric* hole by the points on input port 1 (from output
 *      port 2 of the polyfile reader).
 * <li> a one-component field-data array named "BoundaryMarkers" specifying
 *      the boundary marker value for each input facet (which may span several
 *      cells in the mesh). Facets are identified by cell pedigree IDs, which
 *      are zero-based and sequential.
 * <li> field-data arrays copied from the point data of input port 2.
 *      If this input data is provided from output port 3 of the vtkPolyFileReader,
 *      then "RegionId", "RegionNumber", and "Attribute" arrays will be present
 *      and hold information about the regions specified.
 * </ul>
 *
 * If triangles have an array matching \a FaceGroupArrayName, then the
 * output graph will be collapsed to indicate relationships between facets
 * rather than triangles. Note that this will cause the algorithm to silently
 * malfunction when the input has inconsistently-oriented triangles with
 * identical facet IDs. For instance, if two adjacent triangles with opposing normals
 * are marked as belonging to the same model facet, the inner shell and outer shell
 * for the facet will be merged into a single shell. For non-manifold geometry,
 * the failures can be difficult to debug. You have been warned.
 *
 * If a polydata input is specified on the second input port,
 * then each of its points are used to identify regions that are holes.
 *
 * If a polydata input is specified on the third input port,
 * then it is assumed to be a set of points that identify properties
 * for the regions containing the respective points.
 * Point data from these points will be added to the output field data.
 */
class VTKSMTKMESHINGEXT_EXPORT vtkDiscoverRegions : public vtkPolyDataAlgorithm
{
public:
  static vtkDiscoverRegions* New();
  vtkTypeMacro(vtkDiscoverRegions,vtkPolyDataAlgorithm);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The name of a cell array defined on the input polydata
  // describing which model facet each cell belongs to.
  //
  // If NULL (the default) <b>or</b> ReportRegionsByModelFace is false
  // (also the default), then each cell defines a separate facet for the
  // purpose of region finding.
  // If an empty string, then all polylines form a single facet.
  // If a valid cell-data array name, then each facet may be composed of
  // multiple triangles as specified by this array.
  //
  // If the latter (it is a valid cell-array), then ReportRegionsByModelFace
  // may be set to true when you wish to reduce the cost of region
  // discovery by only tracking regions assigned to model faces (no larger
  // than and often significantly less than the number of triangles).
  vtkGetStringMacro(ModelFaceArrayName);
  vtkSetStringMacro(ModelFaceArrayName);

  // Description:
  // The name of a cell array defined on the input polydata
  // describing a grouping of model facets (not cells).
  // Typically, this grouping is used to apply boundary conditions.
  //
  // If present, the named array will be placed on the output
  // polydata's field data. It may be modified by adding entries
  // to account for model facets which are oversubscribed (i.e., when
  // a model facet has triangles that do not all bound the same
  // two regions -- forcing the model facet to be split).
  //
  // The default is "FaceGroups".
  vtkGetStringMacro(FaceGroupArrayName);
  vtkSetStringMacro(FaceGroupArrayName);

  // Description:
  // The name of a point-data array defined on the *third* input polydata (port 2)
  // containing a unique ID for the region containing the corresponding point.
  //
  // If NULL (the default), then the third input to the filter is ignored.
  vtkGetStringMacro(RegionGroupArrayName);
  vtkSetStringMacro(RegionGroupArrayName);

  // Description:
  // Set/get whether to group shells by model face IDs instead of input
  // dataset cell IDs. If true, ModelFaceArrayName must be set to the
  // name of a valid cell-data vtkIdTypeArray containing a model-face ID
  // for each triangle.
  //
  // The default is false.
  vtkGetMacro(ReportRegionsByModelFace,int);
  vtkSetMacro(ReportRegionsByModelFace,int);
  vtkBooleanMacro(ReportRegionsByModelFace,int);

  // Description:
  // Set/get whether the filter should generate a point interior to each
  // discovered region.
  // The default is false.
  //
  // If true, rays are fired from the midpoints of edges/faces bounding
  // a region until none of the intersections with the remaining faces
  // are near-degenerate (nearly tangent to the ray or near the boundary
  // of the intersected cell) and exactly one side of the ray (forward
  // or backward) has an odd number of intersections with the shell
  // owning the start edge. The midpoint between the ray source and the
  // first intersection on the odd side is interior to the region by
  // definition.
  vtkSetMacro(GenerateRegionInteriorPoints,int);
  vtkGetMacro(GenerateRegionInteriorPoints,int);
  vtkBooleanMacro(GenerateRegionInteriorPoints,int);

protected:
  vtkDiscoverRegions();
  virtual ~vtkDiscoverRegions();

  virtual int FillInputPortInformation(
    int port, vtkInformation* info);

  virtual int FillOutputPortInformation(
    int port, vtkInformation* info);

  virtual int RequestData(
    vtkInformation* req,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);

  int ReportRegionsByModelFace;
  int GenerateRegionInteriorPoints;
  char* ModelFaceArrayName;
  char* FaceGroupArrayName;
  char* RegionGroupArrayName;

private:
  vtkDiscoverRegions(const vtkDiscoverRegions&); // Not implemented.
  void operator = (const vtkDiscoverRegions&); // Not implemented.
};
  } // namespace vtk
} // namespace smtk

#endif // __vtkDiscoverRegions_h
