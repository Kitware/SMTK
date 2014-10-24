//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelEdgeMeshClient - Mesh representation for a vtkModelEdge
// .SECTION Description
// Mesh representation for a vtkModelEdge.  The smaller value for the
// edge length is the one used for the global vs. local values.
// The values are absolute values.  BuildModelEntityMesh() gets called
// if the used edge length parameter changes.  If the edge gets meshed,
// all adjacent model faces ...

#ifndef __vtkCMBModelEdgeMeshClient_h
#define __vtkCMBModelEdgeMeshClient_h

#include "vtkCMBModelEdgeMesh.h"
#include "cmbSystemConfig.h"

class vtkModelEdge;
class vtkCMBModelVertexMesh;

class VTK_EXPORT vtkCMBModelEdgeMeshClient : public vtkCMBModelEdgeMesh
{
public:
  static vtkCMBModelEdgeMeshClient* New();
  vtkTypeMacro(vtkCMBModelEdgeMeshClient,vtkCMBModelEdgeMesh);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the local mesh length for both the client and the server
  // but do not initiate edge meshing.  This is done on both the
  // client and server to make sure they are consistent in case
  // we do model modifications.
  virtual bool SetLocalLength(double len);

protected:
  vtkCMBModelEdgeMeshClient();
  virtual ~vtkCMBModelEdgeMeshClient();
  bool SendLengthToServer();
  friend class vtkCMBMeshClient;

  // Description:
  // This method builds the model entity's mesh without checking
  // the parameters.  Returns true if the operation succeeded as
  // desired.  This includes deleting the mesh if the mesh
  // parameters go from valid to invalid values (i.e. a parameter set to 0).
  bool BuildMesh(bool meshHigherDimensionalEntities);

private:
  vtkCMBModelEdgeMeshClient(const vtkCMBModelEdgeMeshClient&);  // Not implemented.
  void operator=(const vtkCMBModelEdgeMeshClient&);  // Not implemented.
};

#endif
