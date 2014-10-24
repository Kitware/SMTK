//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkDiscreteModelFace - A model face based on a polydata representation.
// .SECTION Description

#ifndef __smtkdiscrete_vtkDiscreteModelFace_h
#define __smtkdiscrete_vtkDiscreteModelFace_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelFace.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include "ModelEdgeHelper.h"


class vtkDiscreteModelEdge;
class vtkDiscreteModelFaceUse;
class vtkDiscreteModelVertex;
class vtkIdList;
class vtkIdTypeArray;
class vtkBitArray;
class vtkPolyData;

class VTKSMTKDISCRETEMODEL_EXPORT vtkDiscreteModelFace : public vtkModelFace,
  public vtkDiscreteModelGeometricEntity
{
public:
  vtkTypeMacro(vtkDiscreteModelFace,vtkModelFace);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkDiscreteModelFace *New();

  // Description:
  // Split this model face based on SplitAngle.  The function
  // fills the created model face UniquePersistentId and edge splitinfo in
  // FaceSplitInfo, and returns true if successful.
  bool Split(double splitAngle,
    std::map<vtkIdType, FaceEdgeSplitInfo>& FaceSplitInfo);

  // Description:
  // Get All/Boundary/Interior point Ids of this model face.
  virtual void GetAllPointIds(vtkIdList* ptsList);
  virtual void GetInteriorPointIds(vtkIdList* ptsList);
  virtual void GetBoundaryPointIds(vtkIdList* ptsList);

  // Description:
  // Mark each index in PointsMask with 0 (out) or 1 (in) for
  // master vtkPoints that are in the vtkDiscreteModelFace grid.
  void GatherAllPointIdsMask(vtkBitArray* pointsMask);

  // Description:
  // Get All/Boundary/Interior point Ids of this model face.
  // Mark each index in PointsMask with 0 (out) or 1 (in) for
  // master vtkPoints that are on the boundary of vtkDiscreteModelFace grid.
  void GatherBoundaryPointIdsMask(vtkBitArray* points);

  // Description:
  // Extract the edges of the face and build new model edges from the extracted
  // edges with proper loop and use information.
  void BuildEdges(bool showEdge, FaceEdgeSplitInfo& splitInfo,
    bool saveLoopInfo=true);

protected:
//BTX
  friend class vtkDiscreteModel;
  friend class vtkCMBMapToCMBModel;
  friend class vtkModelBCGridRepresentation;
//ETX
  vtkDiscreteModelFace();
  virtual ~vtkDiscreteModelFace();

  // Description:
  // Build a new model face from the cells listed in CellIds.
  // The Ids listed in CellIds are with respect to the master grid.
  // The 'saveLoopForExistingFace' argument is a flag to save loopinfo for
  // existing face. This is used for cases like SplitWithFeatureAngle, which
  // may result in many new faces from existing face, and we only want to cache
  // the loopinfo for existing face after the last new face is created.
  vtkDiscreteModelFace* BuildFromExistingModelFace(
    vtkIdList* cellIds, FaceEdgeSplitInfo& splitInfo,
    bool saveLoopForExistingFace);

  // Description:
  // Invoked from BuildFromExistingModelFace when existing model face
  // has edges, and splitting that face may need split its edges too.
  void SplitEdges(vtkDiscreteModelFace* newModelFace, FaceEdgeSplitInfo& splitInfo);

  friend class vtkSelectionSplitOperator;
  friend class vtkCMBIncorporateMeshOperator;

  virtual vtkModelEntity* GetThisModelEntity();
  virtual bool Destroy();

  // Description:
  // Extract the edges of the face and return the information
  // as polydata.
  void ExtractEdges(vtkPolyData* result);

  // Description:
  // Takes in a mesh facet and 2 points that represent an edge on the facet
  // it returns a string that represents an encoded representation of the
  // other model faces along that edge
  std::string EncodeModelFaces(vtkIdType facetId, vtkIdType v0, vtkIdType v1);

  void WalkLoop(vtkIdType startingEdge, vtkPolyData *edges,
                std::vector<bool> &visited,
                vtkIdTypeArray *facetIds,
                NewModelEdgeInfo &newEdgesInfo,
                LoopInfo &loopInfo);
  void CreateModelEdges(NewModelEdgeInfo &newEdgesInfo,
    std::map<int, vtkDiscreteModelEdge*> &newEdges,
    bool bShow, FaceEdgeSplitInfo& splitInfo);

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

private:
  vtkDiscreteModelFace(const vtkDiscreteModelFace&);  // Not implemented.
  void operator=(const vtkDiscreteModelFace&);  // Not implemented.

  void CreateModelFaceUses();
};

#endif

