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
// .NAME vtkCmbModelFaceMesh - Mesh representation for a vtkModelFace
// .SECTION Description
// Mesh representation for a vtkModelFace.  The smaller value for
// maximum area and larger value for minimum angle wins for local
// vs. global comparisons.  BuildModelEntityMesh() is explicitly
// called whenever changing mesh parameters
// will trigger meshing.  Absolute values are used for area.

#ifndef __vtkCmbModelFaceMesh_h
#define __vtkCmbModelFaceMesh_h

#include "vtkCmbModelEntityMesh.h"

class vtkModelFace;
class vtkCmbModelVertexMesh;

class VTK_EXPORT vtkCmbModelFaceMesh : public vtkCmbModelEntityMesh
{
public:
  static vtkCmbModelFaceMesh* New();
  vtkTypeRevisionMacro(vtkCmbModelFaceMesh,vtkCmbModelEntityMesh);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkModelGeometricEntity* GetModelGeometricEntity();

  void Initialize(vtkCmbMesh* mesh, vtkModelFace* face);

  // Description:
  // BuildModelEntityMesh will generate a mesh for the associated
  // model entity.  If meshHigherDimensionalEntities is set to true
  // it will also mesh any higher dimensional entities which need
  // to be meshed because of this object getting meshed.
  bool BuildModelEntityMesh(bool meshHigherDimensionalEntities);

  // Description:
  // Set/get the model face maximum triangle area.  If it is 0 it
  // indicates that it is not set.  If the global max area
  // is smaller than this value then that value will be used
  // when generating the surface mesh.
  vtkGetMacro(MaximumArea, double);
  vtkSetClampMacro(MaximumArea, double, 0, VTK_LARGE_FLOAT);

  // Description:
  // Set/get the model face minimum triangle angle.  If it is 0 it
  // indicates that it is not set.  If the global min angle
  // is smaller than this value then that value will be used
  // when generating the surface mesh.
  vtkGetMacro(MinimumAngle, double);
  vtkSetClampMacro(MinimumAngle, double, 0, VTK_LARGE_FLOAT);

protected:
  vtkCmbModelFaceMesh();
  virtual ~vtkCmbModelFaceMesh();

  // Description:
  // This method builds the model entity's mesh without checking
  // the parameters.
  bool BuildMesh(bool meshHigherDimensionalEntities);

private:
  vtkCmbModelFaceMesh(const vtkCmbModelFaceMesh&);  // Not implemented.
  void operator=(const vtkCmbModelFaceMesh&);  // Not implemented.

  vtkModelFace* ModelFace;
  double MaximumArea;
  double MinimumAngle;
};

#endif
