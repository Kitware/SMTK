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
// vs. global comparisons.  Absolute values are used for area.

#ifndef __vtkCmbModelFaceMesh_h
#define __vtkCmbModelFaceMesh_h

#include "vtkCmbModelEntityMesh.h"
#include "CmbFaceMeshHelper.h"
#include "cmbSystemConfig.h"

class vtkModelFace;
class vtkCmbModelVertexMesh;

class VTK_EXPORT vtkCmbModelFaceMesh : public vtkCmbModelEntityMesh
{
public:
  vtkTypeMacro(vtkCmbModelFaceMesh,vtkCmbModelEntityMesh);
  void PrintSelf(ostream& os, vtkIndent indent);
//BTX
  virtual vtkModelGeometricEntity* GetModelGeometricEntity();

  virtual void Initialize(vtkCmbMesh* mesh, vtkModelFace* face);
//ETX

  // Description:
  // BuildModelEntityMesh will generate a mesh for the associated
  // model entity.  If meshHigherDimensionalEntities is set to true
  // it will also mesh any higher dimensional entities which need
  // to be meshed because of this object getting meshed.  Returns
  // true if the operation succeeded as desired.  This includes
  // deleting the mesh if the mesh parameters go from valid
  // to invalid values (i.e. a parameter set to 0).
  bool BuildModelEntityMesh(bool meshHigherDimensionalEntities);

  // Description:
  // Set/get the model face minimum triangle angle.  If it is 0 it
  // indicates that it is not set.  If the global min angle
  // is smaller than this value then that value will be used
  // when generating the surface mesh.  Note that the angle
  // is in degrees.  The 33 maximum value is imposed by
  // the triangle mesher.
  vtkGetMacro(MinimumAngle, double);
  vtkSetClampMacro(MinimumAngle, double, 0, 33);

  vtkGetMacro(MeshedMinimumAngle, double);

  // Description:
  // Get the desired length the model face will be meshed with.
  // 0 indicates no mesh length has been set.
  double GetActualLength();

  // Description:
  // Get the actual minimum angle the model face will be meshed with.
  // 0 indicates no minimum area has been set.
  double GetActualMinimumAngle();

  virtual bool SetLocalMinimumAngle(double angle)=0;

  vtkGetObjectMacro(ModelFace, vtkModelFace);

  // Description:
  // Return true if the model entity should have a mesh
  // and false otherwise.
  virtual bool IsModelEntityMeshed()
  {
    return (this->GetMeshedLength()> 0. && this->MeshedMinimumAngle > 0.);
  }

protected:
  vtkCmbModelFaceMesh();
  virtual ~vtkCmbModelFaceMesh();

  // Description:
  // Set the MeshedMinimumAngle.  This is protected so that derived
  // classes can use this method.
  vtkSetClampMacro(MeshedMinimumAngle, double, 0, VTK_FLOAT_MAX);

  // Description:
  // This method builds the model entity's mesh without checking
  // the parameters.  Returns true if the operation succeeded as
  // desired.  This includes deleting the mesh if the mesh
  // parameters go from valid to invalid values (i.e. a parameter set to 0).
  virtual bool BuildMesh(bool meshHigherDimensionalEntities) = 0;

  bool CreateMeshInfo();
  bool Triangulate(vtkPolyData *mesh);

private:
  vtkCmbModelFaceMesh(const vtkCmbModelFaceMesh&);  // Not implemented.
  void operator=(const vtkCmbModelFaceMesh&);  // Not implemented.

  vtkModelFace* ModelFace;
  double MinimumAngle; // in degrees

  // Description:
  // The mesh parameters the last time the model face was meshed.
  // If a value is 0 it indicates that the model face has no mesh.
  double MeshedMinimumAngle;
};

#endif
