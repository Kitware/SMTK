//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelFace - Abstract model face class.
// .SECTION Description
// Topology conventions: for 2D faces, faceuse 0 points in negative z-dir
// faceuse 1 points in positive z-dir.  Outer loop edgeuse directions are
// in counterclockwise direction with respect to the faceuse and inner
// loop edgeuse directions are in clockwise direction with respect to
// the faceuse.  outerloop directions are opposite (e.g. eu1+, eu2-, eu3+
// for faceuse1 will have eu1-, eu3-, eu2+).

#ifndef __smtkdiscrete_vtkModelFace_h
#define __smtkdiscrete_vtkModelFace_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelGeometricEntity.h"


#include <set>
#include <vector>

class vtkModel;
class vtkModelEdge;
class vtkModelEdgeUse;
class vtkModelFaceUse;
class vtkModelItemIterator;
class vtkModelLoopUse;
class vtkModelRegion;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelFace : public vtkModelGeometricEntity
{
public:
  vtkTypeMacro(vtkModelFace,vtkModelGeometricEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int GetType();

  // Description:
  // Return the vtkModelFaceUse in the inputted direction.
  vtkModelFaceUse* GetModelFaceUse(int direction);

  // Description:
  // Get the number of adjacent model regions.
  int GetNumberOfModelRegions();

  // Description:
  // Return the vtkModelRegion on side direction of the model face.
  // Returns NULL if no Region exists on that side.
  vtkModelRegion* GetModelRegion(int direction);

  // Description:
  // Get the number of unique model edges adjacent to this model face.
  int GetNumberOfModelEdges();

  // Description:
  // Get the all the model edge/vertex Ids adjacent to this model face.
  void GetModelEdgeIds(std::set<vtkIdType>& edgeIds);
  void GetModelVertexIds(std::set<vtkIdType>& verIds);

  // Description:
  // Get the model edges adjacent to this model face.
  void GetModelEdges(std::vector<vtkModelEdge*>& edges);

  // Description:
  // Create an iterator to loop over the adjacent model edges.  It
  // can have a model edge listed more than once so it may have
  // more objects in it than this->GetNumberOfModeledges().
  vtkModelItemIterator* NewAdjacentModelEdgeIterator();

  using Superclass::Initialize;

  // Description:
  // Assume that the loop is the face's outer loop
  // Initializes the model face by creating the outer loop for it.
  // The edges need to be passed in in counterclockwise order.
  virtual void Initialize(int NumEdges, vtkModelEdge** edges,
                          int* edgeDirections, vtkIdType modelFaceId);

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

  // Description:
  // Adds an edge loop to a model face
  virtual void AddLoop(int numEdges, vtkModelEdge** edges,
                       int* edgeDirections);

  // Description:
  // Destroy all loopuses of a model face
  virtual bool DestroyLoopUses();

  // Description:
  // Gets the number of holes in the model face.  This is NOT the
  // number of loops-1 since an "inner" loop could be degenerate
  // and isn't considered a hole.
  int GetNumberOfHoles();

  // Description:
  // Gets the number of degenerate loops.  These are loops which
  // use each model edge twice and would have no area associated
  // with any model face that they could be on the outer boundary of.
  int GetNumberOfDegenerateLoops();

protected:
  vtkModelFace();
  virtual ~vtkModelFace();

  virtual bool IsDestroyable();
  virtual bool Destroy();

  // Description:
  // Helper function that combines "end" of edgeUse1 with "beginning" of edgeUse2.
  // Edge uses are set up so that second vertexuse of edgeUse1 is merged with
  // the first vertex use of edgeUse2.
  void CombineModelVertexUses(vtkModelEdgeUse* edgeUse1,
                              vtkModelEdgeUse* edgeUse2);

//BTX
  friend class vtkModel;
//ETX

private:
  vtkModelFace(const vtkModelFace&);  // Not implemented.
  void operator=(const vtkModelFace&);  // Not implemented.
};

#endif
