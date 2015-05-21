//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelEdge - Abstract generic model entity class.
// .SECTION Description
// Topologically, edge use 1 is in the same direction of the model edge
// and edge use 0 is in the opposite direction.

#ifndef __smtkdiscrete_vtkModelEdge_h
#define __smtkdiscrete_vtkModelEdge_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelGeometricEntity.h"


class vtkModelEdgeUse;
class vtkModelItemIterator;
class vtkModelVertex;
class vtkModelVertexUse;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelEdge : public vtkModelGeometricEntity
{
public:
  vtkTypeMacro(vtkModelEdge,vtkModelGeometricEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int GetType();

  int GetNumberOfModelEdgeUses();
  vtkModelEdgeUse* GetModelEdgeUse(int i);
  vtkModelItemIterator* NewModelEdgeUseIterator();

  // Description:
  // Get information about the model vertex use and adjacent
  // model vertex.  Note that the model vertex may be repeated
  // but the model vertex uses will be different. which 0 is for the first
  // vertex and non-zero for the second vertex.  Note that
  // there may be 0, 1, or 2 adjacent vertices.
  int GetNumberOfModelVertexUses();
  vtkModelVertex* GetAdjacentModelVertex(int which);

  int GetNumberOfAdjacentModelFaces();
  vtkModelItemIterator* NewAdjacentModelFaceIterator();

  using Superclass::Initialize;
  virtual void Initialize(vtkModelVertex* vertex0, vtkModelVertex* vertex1,
                          vtkIdType edgeId);

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

  // Description:
  // Split the edge with given vertex and/or edge. This is mainly used
  // to update the topology associations, for example the edge uses,
  // for the related oldEdge, newEdge, and newVert after Split.
  // It is particualarly useful for updating client model without geometry.
  void SplitModelEdge(vtkModelVertex* newVertex, vtkModelEdge* newEdge);
  bool SplitModelEdgeLoop(vtkModelVertex* Vertex);

protected:
  vtkModelEdge();
  virtual ~vtkModelEdge();

  // Description:
  // Build a model edge use pair (2 model edge uses).
  // Returns the second model edge use in the same direction.  Use
  // vtkModelEdgeUse::GetPairedModelEdgeUse() to get the other one.
  vtkModelEdgeUse* BuildModelEdgeUsePair();

  void DestroyModelEdgeUse(vtkModelEdgeUse* edgeUse);
  void SplitModelEdgeUse(
    vtkModelEdgeUse* firstEdgeUse, vtkModelEdgeUse* secondEdgeUse,
    vtkModelVertexUse* vertexUse0, vtkModelVertexUse* vertexUse1,
    vtkModelVertexUse* vertexUse2);

//BTX
  friend class vtkModel;
  friend class vtkModelFace;
  friend class vtkModelLoopUse;
//ETX

private:
  vtkModelEdge(const vtkModelEdge&);  // Not implemented.
  void operator=(const vtkModelEdge&);  // Not implemented.
};

#endif

