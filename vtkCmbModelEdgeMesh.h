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
// .NAME vtkCmbModelEdgeMesh - Abstract mesh representation for a vtkModelEdge
// .SECTION Description
// Abstract mesh representation for a vtkModelEdge.  The concrete classes
// are for the client and the server.  The smaller value for the
// edge length is the one used for the global vs. local values.
// The values are absolute values.

#ifndef __vtkCmbModelEdgeMesh_h
#define __vtkCmbModelEdgeMesh_h

#include "vtkCmbModelEntityMesh.h"
#include "cmbSystemConfig.h"

class vtkModelEdge;
class vtkCmbModelVertexMesh;

class VTK_EXPORT vtkCmbModelEdgeMesh : public vtkCmbModelEntityMesh
{
public:
  vtkTypeMacro(vtkCmbModelEdgeMesh,vtkCmbModelEntityMesh);
  void PrintSelf(ostream& os, vtkIndent indent);
//BTX
  virtual vtkModelGeometricEntity* GetModelGeometricEntity();

  // Description:
  // Initialize the model edge mesh.  This sets Length to 0
  // and removes the polydata/proxy.
  virtual void Initialize(vtkCmbMesh* mesh, vtkModelEdge* edge);
//ETX
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
  vtkCmbModelVertexMesh* GetAdjacentModelVertexMesh(int which);

  vtkGetObjectMacro(ModelEdge, vtkModelEdge);

  // Description:
  // Get the actual length the model edge will be meshed with.
  // 0 indicates no length has been set.
  double GetActualLength();

protected:
  vtkCmbModelEdgeMesh();
  virtual ~vtkCmbModelEdgeMesh();

  // Description:
  // This method builds the model entity's mesh without checking
  // the parameters.  Returns true if the operation succeeded as
  // desired.  This includes deleting the mesh if the mesh
  // parameters go from valid to invalid values (i.e. a parameter set to 0).
  virtual bool BuildMesh(bool meshHigherDimensionalEntities) = 0;

private:
  vtkCmbModelEdgeMesh(const vtkCmbModelEdgeMesh&);  // Not implemented.
  void operator=(const vtkCmbModelEdgeMesh&);  // Not implemented.

  vtkModelEdge* ModelEdge;

};

#endif

