//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelEdgeMesh - Abstract mesh representation for a vtkModelEdge
// .SECTION Description
// Abstract mesh representation for a vtkModelEdge.  The concrete classes
// are for the client and the server.  The smaller value for the
// edge length is the one used for the global vs. local values.
// The values are absolute values.

#ifndef __vtkCMBModelEdgeMesh_h
#define __vtkCMBModelEdgeMesh_h

#include "cmbSystemConfig.h"
#include "vtkCMBModelEntityMesh.h"

class vtkModelEdge;
class vtkCMBModelVertexMesh;

class VTK_EXPORT vtkCMBModelEdgeMesh : public vtkCMBModelEntityMesh
{
public:
  vtkTypeMacro(vtkCMBModelEdgeMesh,vtkCMBModelEntityMesh);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkModelGeometricEntity* GetModelGeometricEntity();

  // Description:
  // Initialize the model edge mesh.  This sets Length to 0
  // and removes the polydata/proxy.
  virtual void Initialize(vtkCMBMesh* mesh, vtkModelEdge* edge);

  // Description:
  // BuildModelEntityMesh will generate a mesh for the associated
  // model entity.  If meshHigherDimensionalEntities is set to true
  // it will also mesh any higher dimensional entities which need
  // to be meshed because of this object getting meshed.  Returns
  // true if the operation succeeded as desired.  This includes
  // deleting the mesh if the mesh parameters go from valid
  // to invalid values (i.e. a parameter set to 0).
  virtual bool BuildModelEntityMesh(bool meshHigherDimensionalEntities);

  // Description:
  // Get the model vertex mesh object associated with which
  // adjacent model vertex.  If there is no adjacent model vertex
  // then this function returns NULL.  If it has only a single
  // adjacent model vertex then it return the same model vertex
  // mesh object for both which = 0 and which = 1.
  vtkCMBModelVertexMesh* GetAdjacentModelVertexMesh(int which);

  vtkGetObjectMacro(ModelEdge, vtkModelEdge);

  // Description:
  // Get the actual length the model edge will be meshed with.
  // 0 indicates no length has been set.
  double GetActualLength();

protected:
  vtkCMBModelEdgeMesh();
  virtual ~vtkCMBModelEdgeMesh();

  // Description:
  // This method builds the model entity's mesh without checking
  // the parameters.  Returns true if the operation succeeded as
  // desired.  This includes deleting the mesh if the mesh
  // parameters go from valid to invalid values (i.e. a parameter set to 0).
  virtual bool BuildMesh(bool meshHigherDimensionalEntities) = 0;

private:
  vtkCMBModelEdgeMesh(const vtkCMBModelEdgeMesh&);  // Not implemented.
  void operator=(const vtkCMBModelEdgeMesh&);  // Not implemented.

  vtkModelEdge* ModelEdge;

};

#endif

