//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelEdgeMeshServer - Mesh representation for a vtkModelEdge
// .SECTION Description
// Mesh representation for a vtkModelEdge.  The smaller value for the
// edge length is the one used for the global vs. local values.
// The values are absolute values.  BuildModelEntityMesh() gets called
// if the used edge length parameter changes.  If the edge gets meshed,
// all adjacent model faces ...

#ifndef __vtkCMBModelEdgeMeshServer_h
#define __vtkCMBModelEdgeMeshServer_h

#include "cmbSystemConfig.h"
#include "vtkCMBModelEdgeMesh.h"

class VTK_EXPORT vtkCMBModelEdgeMeshServer : public vtkCMBModelEdgeMesh
{
public:
  static vtkCMBModelEdgeMeshServer* New();
  vtkTypeMacro(vtkCMBModelEdgeMeshServer,vtkCMBModelEdgeMesh);
  void PrintSelf(ostream& os, vtkIndent indent);
  // Description:
  // Set the local mesh length on the entity.
  virtual bool SetLocalLength(double len);

protected:
  vtkCMBModelEdgeMeshServer();
  virtual ~vtkCMBModelEdgeMeshServer();

  // Description:
  // Set/Get whether or not to use length along edge (as opposed to distance
  // between current mesh point and the next) when computing the next mesh pt.
  // Defaults to "true".
  vtkBooleanMacro(UseLengthAlongEdge, bool);
  vtkSetMacro(UseLengthAlongEdge, bool);
  vtkGetMacro(UseLengthAlongEdge, bool);

  // Description:
  // This method builds the model entity's mesh without checking
  // the parameters.  Returns true if the operation succeeded as
  // desired.  This includes deleting the mesh if the mesh
  // parameters go from valid to invalid values (i.e. a parameter set to 0).
  bool BuildMesh(bool meshHigherDimensionalEntities);

private:
  bool UseLengthAlongEdge;
  vtkCMBModelEdgeMeshServer(const vtkCMBModelEdgeMeshServer&);  // Not implemented.
  void operator=(const vtkCMBModelEdgeMeshServer&);  // Not implemented.
};

#endif
