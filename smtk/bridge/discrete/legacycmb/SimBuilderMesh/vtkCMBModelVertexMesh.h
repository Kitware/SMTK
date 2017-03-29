//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelVertexMesh - Mesh representation for a vtkModelVertex
// .SECTION Description

#ifndef __vtkCMBModelVertexMesh_h
#define __vtkCMBModelVertexMesh_h

#include "cmbSystemConfig.h"
#include "vtkCMBModelEntityMesh.h"

class vtkModelVertex;

class VTK_EXPORT vtkCMBModelVertexMesh : public vtkCMBModelEntityMesh
{
public:
  static vtkCMBModelVertexMesh* New();
  vtkTypeMacro(vtkCMBModelVertexMesh,vtkCMBModelEntityMesh);
  void PrintSelf(ostream& os, vtkIndent indent);
  // Description:
  // Get the actual length the model edge will be meshed with.
  // 0 indicates no length has been set.
  virtual double GetActualLength()
  {
    return this->GetLength();
  }

  // Description:
  // Set the local mesh length on the entity.
  virtual bool SetLocalLength(double len)
  {
    this->SetLength(len);
    return true;
  }

  virtual vtkModelGeometricEntity* GetModelGeometricEntity();

  void Initialize(vtkCMBMesh* mesh, vtkModelVertex* vertex);

  // Description:
  // BuildModelEntityMesh will generate a mesh for the associated
  // model entity.  If meshHigherDimensionalEntities is set to true
  // it will also mesh any higher dimensional entities which need
  // to be meshed because of this object getting meshed.
  bool BuildModelEntityMesh(bool meshHigherDimensionalEntities);

  // Description:
  // Return true if the model entity should have a mesh
  // and false otherwise.
  virtual bool IsModelEntityMeshed()
  {
    return false;
  }

protected:
  vtkCMBModelVertexMesh();
  virtual ~vtkCMBModelVertexMesh();

private:
  vtkCMBModelVertexMesh(const vtkCMBModelVertexMesh&);  // Not implemented.
  void operator=(const vtkCMBModelVertexMesh&);  // Not implemented.

  vtkModelVertex* ModelVertex;
};

#endif

