//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkSeedGrowSelectionFilter
// .SECTION Description
#ifndef __vtkSeedGrowSelectionFilter_h
#define __vtkSeedGrowSelectionFilter_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkSelectionAlgorithm.h"
#include <set>

class DiscreteMesh;
class vtkDiscreteModelWrapper;
class vtkIdList;
class vtkIdTypeArray;
class vtkIntArray;
class vtkPolyData;

class SMTKDISCRETESESSION_EXPORT vtkSeedGrowSelectionFilter : public vtkSelectionAlgorithm
{
public:
  static vtkSeedGrowSelectionFilter* New();
  vtkTypeMacro(vtkSeedGrowSelectionFilter, vtkSelectionAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the vtkSelection object used for selecting the
  // mesh facets.
  void SetSelectionConnection(vtkAlgorithmOutput* algOutput);
  void SetInputSelection(vtkSelection* selection);

  // Description:
  // Removes all inputs from input port 0.
  void RemoveAllSelectionsInputs() { this->SetInputConnection(0, 0); }

  // Description:
  // Specify the angle that defines a sharp edge. If the difference in
  // angle across neighboring polygons is greater than this value, the
  // shared edge is considered "sharp".
  vtkSetClampMacro(FeatureAngle, double, 0.0, 180.0);
  vtkGetMacro(FeatureAngle, double);

  // Description:
  // Macro to set/get CellId and FaceId.  The faceId is the UniqueConsistentId.
  // The CellId is the id of the cell on
  // the model face to start the grow operation from.
  void SetFaceCellId(vtkIdType faceId, vtkIdType cellId);
  vtkGetMacro(CellId, vtkIdType);
  vtkGetMacro(ModelFaceId, vtkIdType);

  // Description:
  // Macro to set/get CellId and FaceId
  // 0, Normal (ignore selection input);
  // 1, Plus (merge result with selection input);
  // 2, Minus (remove result from selection input);
  vtkSetClampMacro(GrowMode, int, 0, 2);
  vtkGetMacro(GrowMode, int);

  // Description:
  // Set/clear the model face Ids array that will be grown upon.
  void SetGrowFaceIds(vtkIdType*);
  void RemoveAllGrowFaceIds();
  void SetGrowFaceIds(const std::set<vtkIdType>& ModelFaceIds);

  // Description:
  // Set/get macros for the vtkDiscreteModelWrapper.
  void SetModelWrapper(vtkDiscreteModelWrapper*);
  vtkGetObjectMacro(ModelWrapper, vtkDiscreteModelWrapper);

protected:
  vtkSeedGrowSelectionFilter();
  ~vtkSeedGrowSelectionFilter();

  double FeatureAngle;
  // Cosine of the feature angle.
  double FeatureAngleCosine;

  // Description:
  // The starting CellId and FaceId of the grow operation
  vtkIdType CellId;
  vtkIdType ModelFaceId;

  // 0, Normal (ignore selection input);
  // 1, Plus (merge result with selection input);
  // 2, Minus (remove result from selection input);
  int GrowMode;

  // Description:
  // A list of cell Ids on the master poly data that are created from the
  // grow operation.
  vtkIdList* GrowCellIds;

  // Description:
  // The vtkDiscreteModelWrapper for the algorithm to extract the model
  // information from.
  vtkDiscreteModelWrapper* ModelWrapper;

  // Description:
  // Iterative algorithm to grow from a passed in cell id in a vtkPolyData.
  // The marked array is used to store whether or not the cell is in the
  // list of "grown" cells.
  void GrowFromCell(
    const DiscreteMesh* mesh, vtkIntArray* marked, vtkIdType cellId, vtkIdTypeArray* selectionList);

  void GrowAndRemoveFromSelection(const DiscreteMesh* mesh, vtkIntArray* marked, vtkIdType cellId,
    vtkIdTypeArray* outSelectionList, vtkSelection* inSelection);

  // Description:
  // Computes the normal of a triangle (or the first 3 points of polygon)
  // with points in ptids and returns the values in normal. We have assumed
  // that all polygons are planar but possibly not convex as per
  // discussions with CMB
  void ComputeNormal(vtkIdList* ptids, const DiscreteMesh* mesh, double normal[3]);

  // Description:
  // We cannot assume that the normals are consistent between
  // model faces.  Because of this we check to see the canonical
  // ordering of the cells' points. PointId1 is the first point
  // along the edge of the first triangle and PointId2 is the second
  // point (canonicall ordered).  NeighborCellIds are the point ids
  // of the neighboring cell (also canonically ordered).  Returns
  // true if they are consistent.
  bool AreCellNormalsConsistent(vtkIdType PointId1, vtkIdType PointId2, vtkIdList* NeighborCellIds);

  // Description:
  // Recursive algorithm to merge which cells have been selected.
  void MergeGrowSelection(vtkSelection*, vtkIntArray*, vtkIdTypeArray* outSelectionList);

  // Description:
  // This is called within ProcessRequest when a request asks the algorithm
  // to do its work. This is the method you should override to do whatever the
  // algorithm is designed to do. This happens during the fourth pass in the
  // pipeline execution process.
  virtual int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);
  virtual int FillInputPortInformation(int port, vtkInformation* info);

private:
  vtkSeedGrowSelectionFilter(const vtkSeedGrowSelectionFilter&); // Not implemented.
  void operator=(const vtkSeedGrowSelectionFilter&);             // Not implemented.

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
