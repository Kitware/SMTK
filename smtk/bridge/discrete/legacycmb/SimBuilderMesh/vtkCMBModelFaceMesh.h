//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelFaceMesh - Mesh representation for a vtkModelFace
// .SECTION Description
// Mesh representation for a vtkModelFace.  The smaller value for
// maximum area and larger value for minimum angle wins for local
// vs. global comparisons.  Absolute values are used for area.

#ifndef __vtkCMBModelFaceMesh_h
#define __vtkCMBModelFaceMesh_h

#include "vtkCMBModelEntityMesh.h"
#include "cmbFaceMeshHelper.h"
#include "cmbSystemConfig.h"

class vtkModelFace;
class vtkCMBModelVertexMesh;

class VTK_EXPORT vtkCMBModelFaceMesh : public vtkCMBModelEntityMesh
{
public:
  vtkTypeMacro(vtkCMBModelFaceMesh,vtkCMBModelEntityMesh);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkModelGeometricEntity* GetModelGeometricEntity();

  virtual void Initialize(vtkCMBMesh* mesh, vtkModelFace* face);

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
  vtkCMBModelFaceMesh();
  virtual ~vtkCMBModelFaceMesh();

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

private:
  vtkCMBModelFaceMesh(const vtkCMBModelFaceMesh&);  // Not implemented.
  void operator=(const vtkCMBModelFaceMesh&);  // Not implemented.

  vtkModelFace* ModelFace;
  double MinimumAngle; // in degrees

  // Description:
  // The mesh parameters the last time the model face was meshed.
  // If a value is 0 it indicates that the model face has no mesh.
  double MeshedMinimumAngle;
};

#endif
