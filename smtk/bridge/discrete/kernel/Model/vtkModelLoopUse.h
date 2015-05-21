//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelLoopUse - Abstract generic model entity class.
// .SECTION Description

#ifndef __smtkdiscrete_vtkModelLoopUse_h
#define __smtkdiscrete_vtkModelLoopUse_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelEntity.h"


class vtkModelEdgeUse;
class vtkModelFace;
class vtkModelItemIterator;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelLoopUse : public vtkModelEntity
{
public:
  vtkTypeMacro(vtkModelLoopUse,vtkModelEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int GetType();

  int GetNumberOfModelEdgeUses();
  vtkModelEdgeUse* GetModelEdgeUse(int index);
  vtkModelItemIterator* NewModelEdgeUseIterator();

  vtkModelFace* GetModelFace();

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

  // Description:
  // Get the index of edgeUse in this loop.  Returns -1
  // if edgeUse is not associated with this loop.
  int GetModelEdgeUseIndex(vtkModelEdgeUse* edgeUse);

  // Description:
  // Get the number of vertices in this loop.  Note that
  // if a vertex is in the loop more than once it is still
  // only counted once.
  int GetNumberOfModelVertices();

protected:
  static vtkModelLoopUse* New();
  vtkModelLoopUse();
  virtual ~vtkModelLoopUse();

  void InsertModelEdgeUse(int Index, vtkModelEdgeUse* edgeUse);

  void RemoveModelEdgeUseAssociation(vtkModelEdgeUse* edgeUse);

//BTX
  friend class vtkDiscreteModelGeometricEntity;
  friend class vtkXMLModelReader;
  friend class vtkModelEdge;
  friend class vtkModelFace;
  friend class vtkModelFaceUse;
  friend class vtkModel;
//ETX

  virtual bool Destroy();

private:
  vtkModelLoopUse(const vtkModelLoopUse&);  // Not implemented.
  void operator=(const vtkModelLoopUse&);  // Not implemented.
};

#endif

