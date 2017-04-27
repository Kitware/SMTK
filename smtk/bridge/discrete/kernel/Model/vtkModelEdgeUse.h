//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelEdgeUse - Model edge use class.
// .SECTION Description
// Model edge use class that directly stores adjacency information
// to 0 to 2 model vertex uses, a model loop use, and another
// model edge use that is in the opposite direction (i.e. they combine
// to be a model edge use pair).

#ifndef __smtkdiscrete_vtkModelEdgeUse_h
#define __smtkdiscrete_vtkModelEdgeUse_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelEntity.h"

class vtkInformationIntegerKey;
class vtkInformationKey;
class vtkModelEdge;
class vtkModelLoopUse;
class vtkModelVertex;
class vtkModelVertexUse;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelEdgeUse : public vtkModelEntity
{
public:
  static vtkModelEdgeUse* New();
  vtkTypeMacro(vtkModelEdgeUse, vtkModelEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int GetType();

  vtkModelEdge* GetModelEdge();

  vtkModelEdgeUse* GetPairedModelEdgeUse();

  vtkModelLoopUse* GetModelLoopUse();

  vtkModelVertexUse* GetModelVertexUse(int i);
  int GetNumberOfModelVertexUses();

  // Description:
  // Get the direction of this edge use with respect to its
  // associated model edge's direction (0 is the opposite direction).
  int GetDirection();

  using Superclass::Initialize;
  void Initialize(vtkModelVertex* vertex0, vtkModelVertex* vertex1, vtkModelEdgeUse* pairedEdgeUse,
    int direction);

  // Description:
  // Static functions for declaring the direction of
  // a vtkModelEdgeUse relative to its vtkModelEdge.
  static vtkInformationIntegerKey* DIRECTION();

  // Description:
  // Destroy the object.  It will still have an association
  // to the "owning" model edge so the model edge must get
  // rid of the associations in order to actually delete it.
  virtual bool Destroy();

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

protected:
  vtkModelEdgeUse();
  virtual ~vtkModelEdgeUse();

  void SetModelVertexUses(vtkModelVertexUse* vertexUse0, vtkModelVertexUse* vertexUse1);

  void SetDirection(int direction);

  friend class vtkModelEdge;
  friend class vtkModelFace;
  friend class vtkModelFaceUse;
  friend class vtkXMLModelReader;
  friend class vtkDiscreteModelGeometricEntity;

private:
  vtkModelEdgeUse(const vtkModelEdgeUse&); // Not implemented.
  void operator=(const vtkModelEdgeUse&);  // Not implemented.
};

#endif
