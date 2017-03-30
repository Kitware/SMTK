//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelVertexUse - Abstract generic model entity class.
// .SECTION Description

#ifndef __smtkdiscrete_vtkModelVertexUse_h
#define __smtkdiscrete_vtkModelVertexUse_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelEntity.h"

class vtkModelEdgeUse;
class vtkModelVertex;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelVertexUse : public vtkModelEntity
{
public:
  static vtkModelVertexUse* New();
  vtkTypeMacro(vtkModelVertexUse, vtkModelEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int GetType();

  vtkModelVertex* GetModelVertex();

  int GetNumberOfModelEdgeUses();
  vtkModelItemIterator* NewModelEdgeUseIterator();

  using Superclass::Initialize;
  void Initialize(vtkModelVertex* vertex);

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

protected:
  vtkModelVertexUse();
  virtual ~vtkModelVertexUse();

  void AddModelEdgeUse(vtkModelEdgeUse* edgeUse);
  void RemoveModelEdgeUse(vtkModelEdgeUse* edgeUse);

  friend class vtkModelEdge;
  friend class vtkModelEdgeUse;

  virtual bool Destroy();
  // for destroying

  friend class vtkModelFace;
  friend class vtkModelFaceUse;
  friend class vtkModelVertex;

private:
  vtkModelVertexUse(const vtkModelVertexUse&); // Not implemented.
  void operator=(const vtkModelVertexUse&);    // Not implemented.
};

#endif
