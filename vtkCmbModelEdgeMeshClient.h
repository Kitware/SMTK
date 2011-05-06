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
// .NAME vtkCmbModelEdgeMeshClient - Mesh representation for a vtkModelEdge
// .SECTION Description
// Mesh representation for a vtkModelEdge.  The smaller value for the
// edge length is the one used for the global vs. local values.
// The values are absolute values.  BuildModelEntityMesh() gets called
// if the used edge length parameter changes.  If the edge gets meshed,
// all adjacent model faces ...

#ifndef __vtkCmbModelEdgeMeshClient_h
#define __vtkCmbModelEdgeMeshClient_h

#include "vtkCmbModelEdgeMesh.h"

class vtkModelEdge;
class vtkCmbModelVertexMesh;

class VTK_EXPORT vtkCmbModelEdgeMeshClient : public vtkCmbModelEdgeMesh
{
public:
  static vtkCmbModelEdgeMeshClient* New();
  vtkTypeRevisionMacro(vtkCmbModelEdgeMeshClient,vtkCmbModelEdgeMesh);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the local mesh length for both the client and the server
  // but do not initiate edge meshing.  This is done on both the
  // client and server to make sure they are consistent in case
  // we do model modifications.
  virtual bool SetLocalLength(double len);

protected:
  vtkCmbModelEdgeMeshClient();
  virtual ~vtkCmbModelEdgeMeshClient();
  bool SendLengthToServer();
  friend class vtkCmbMeshClient;

  // Description:
  // This method builds the model entity's mesh without checking
  // the parameters.  Returns true if the operation succeeded as
  // desired.  This includes deleting the mesh if the mesh
  // parameters go from valid to invalid values (i.e. a parameter set to 0).
  bool BuildMesh(bool meshHigherDimensionalEntities);

private:
  vtkCmbModelEdgeMeshClient(const vtkCmbModelEdgeMeshClient&);  // Not implemented.
  void operator=(const vtkCmbModelEdgeMeshClient&);  // Not implemented.
};

#endif
