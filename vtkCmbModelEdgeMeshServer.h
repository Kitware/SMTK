/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
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
// .NAME vtkCmbModelEdgeMeshServer - Mesh representation for a vtkModelEdge
// .SECTION Description
// Mesh representation for a vtkModelEdge.  The smaller value for the
// edge length is the one used for the global vs. local values.
// The values are absolute values.  BuildModelEntityMesh() gets called
// if the used edge length parameter changes.  If the edge gets meshed,
// all adjacent model faces ...

#ifndef __vtkCmbModelEdgeMeshServer_h
#define __vtkCmbModelEdgeMeshServer_h

#include "vtkCmbModelEdgeMesh.h"
#include "cmbSystemConfig.h"

class VTK_EXPORT vtkCmbModelEdgeMeshServer : public vtkCmbModelEdgeMesh
{
public:
  static vtkCmbModelEdgeMeshServer* New();
  vtkTypeMacro(vtkCmbModelEdgeMeshServer,vtkCmbModelEdgeMesh);
  void PrintSelf(ostream& os, vtkIndent indent);
  // Description:
  // Set the local mesh length on the entity.
  virtual bool SetLocalLength(double len);

protected:
  vtkCmbModelEdgeMeshServer();
  virtual ~vtkCmbModelEdgeMeshServer();

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
  vtkCmbModelEdgeMeshServer(const vtkCmbModelEdgeMeshServer&);  // Not implemented.
  void operator=(const vtkCmbModelEdgeMeshServer&);  // Not implemented.
};

#endif
