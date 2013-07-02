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
// .NAME vtkModelEdge - Abstract generic model entity class.
// .SECTION Description
// Topologically, edge use 1 is in the same direction of the model edge
// and edge use 0 is in the opposite direction.

#ifndef __vtkModelEdge_h
#define __vtkModelEdge_h

#include "vtkDiscreteModelModule.h" // For export macro
#include "vtkModelGeometricEntity.h"

class vtkModelEdgeUse;
class vtkModelItemIterator;
class vtkModelVertex;
class vtkModelVertexUse;

class VTKDISCRETEMODEL_EXPORT vtkModelEdge : public vtkModelGeometricEntity
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

  virtual void Initialize(vtkModelVertex* Vertex0, vtkModelVertex* Vertex1,
                          vtkIdType EdgeId);

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

  void DestroyModelEdgeUse(vtkModelEdgeUse* EdgeUse);

  void SplitModelEdgeUse(
    vtkModelEdgeUse* FirstEdgeUse, vtkModelEdgeUse* SecondEdgeUse,
    vtkModelVertexUse* VertexUse0, vtkModelVertexUse* VertexUse1,
    vtkModelVertexUse* VertexUse2);


//BTX
  friend class vtkModel;
  friend class vtkModelFace;
//ETX

private:
  vtkModelEdge(const vtkModelEdge&);  // Not implemented.
  void operator=(const vtkModelEdge&);  // Not implemented.
};

#endif

