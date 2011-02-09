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
// .NAME vtkCmbModelFaceMeshServer - Mesh representation for a vtkModelFace
// .SECTION Description
// Mesh representation for a vtkModelFace.  The smaller value for
// maximum area and larger value for minimum angle wins for local
// vs. global comparisons.  BuildModelEntityMesh() is explicitly
// called whenever changing mesh parameters
// will trigger meshing.  Absolute values are used for area.

#ifndef __vtkCmbModelFaceMeshServer_h
#define __vtkCmbModelFaceMeshServer_h

#include "vtkCmbModelFaceMesh.h"

class vtkModelFace;
class vtkCmbModelVertexMesh;

class VTK_EXPORT vtkCmbModelFaceMeshServer : public vtkCmbModelFaceMesh
{
public:
  static vtkCmbModelFaceMeshServer* New();
  vtkTypeRevisionMacro(vtkCmbModelFaceMeshServer,vtkCmbModelFaceMesh);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkCmbModelFaceMeshServer();
  virtual ~vtkCmbModelFaceMeshServer();

  // Description:
  // This method builds the model entity's mesh without checking
  // the parameters.
  bool BuildMesh(bool meshHigherDimensionalEntities);

private:
  vtkCmbModelFaceMeshServer(const vtkCmbModelFaceMeshServer&);  // Not implemented.
  void operator=(const vtkCmbModelFaceMeshServer&);  // Not implemented.
};

#endif
