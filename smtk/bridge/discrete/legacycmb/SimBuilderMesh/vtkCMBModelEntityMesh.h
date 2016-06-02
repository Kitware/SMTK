//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelEntityMesh - Mesh representation for a vtkModelGeometricEntity
// .SECTION Description
// Class is abstract.

#ifndef __vtkCMBModelEntityMesh_h
#define __vtkCMBModelEntityMesh_h

#include <vtkObject.h>
#include "cmbSystemConfig.h"

#include <vtkModelGeometricEntity.h>
#include <vtkWeakPointer.h>

class vtkCMBMesh;
class vtkPolyData;

class VTK_EXPORT vtkCMBModelEntityMesh : public vtkObject
{
public:
  vtkTypeMacro(vtkCMBModelEntityMesh,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get visible (true is visible).
  vtkSetMacro(Visible, bool);
  vtkGetMacro(Visible, bool);

  virtual vtkModelGeometricEntity* GetModelGeometricEntity() = 0;

  vtkGetObjectMacro(MasterMesh, vtkCMBMesh);

  // Description:
  // Get the model entity's analysis mesh.  On the server it's a
  // vtkPolyData and on the client it's NULL.
  vtkGetObjectMacro(ModelEntityMesh, vtkPolyData);

  // Description:
  // BuildModelEntityMesh will generate a mesh for the associated
  // model entity.  If meshHigherDimensionalEntities is set to true
  // it will also mesh any higher dimensional entities which need
  // to be meshed because of this object getting meshed.  Returns
  // true if the operation succeeded as desired.  This includes
  // deleting the mesh if the mesh parameters go from valid
  // to invalid values (i.e. a parameter set to 0).
  virtual bool BuildModelEntityMesh(
    bool meshHigherDimensionalEntities) = 0;

  // Description:
  // Set/get the model entity mesh length.  If it is 0 it
  // indicates that it is not set.  If the global entity length
  // is smaller than this value then that value will be used
  // when generating the model entity mesh.  If the actual
  // used length gets modified then the model entity
  // automatically gets remeshed.
  vtkGetMacro(Length, double);
  vtkSetClampMacro(Length, double, 0, VTK_FLOAT_MAX);

  // Description:
  // Get the actual length used to mesh this entity (0 indicates
  // that no valid length exists and therefore there should
  // not be a mesh of this model entity.
  vtkGetMacro(MeshedLength, double);

  // Description:
  // Get the actual length the model edge will be meshed with.
  // 0 indicates no length has been set.
  virtual double GetActualLength() = 0;

  // Description:
  // Return true if the model entity should have a mesh
  // and false otherwise.
  virtual bool IsModelEntityMeshed()
  {
    return (this->MeshedLength > 0.);
  }

  // Description:
  // Set the local mesh length on the entity.
  virtual bool SetLocalLength(double len) = 0;

protected:
  vtkCMBModelEntityMesh();
  virtual ~vtkCMBModelEntityMesh();

  // Description:
  // Mesh is reference counted.  On the server mesh is
  // a vtkPolyData and on the client it's NULL.
  void SetModelEntityMesh(vtkPolyData* mesh);

  // Description:
  // Mesh is not reference counted here.
  vtkSetMacro(MasterMesh, vtkCMBMesh*);

  // Description:
  // Set the MeshedLength.  This is protected so that derived
  // classes can use this method.
  vtkSetClampMacro(MeshedLength, double, 0, VTK_FLOAT_MAX);


private:
  bool Visible;
  vtkCMBMesh* MasterMesh;
  vtkPolyData* ModelEntityMesh;

  // Description:
  // The set edge cell length for this model entity.  If the global
  // length is smaller than this then that value is used for
  // meshing (ignoring 0/unset length).
  double Length;

  // Description:
  // The meshed length of the current mesh if it exists.
  double MeshedLength;

  vtkCMBModelEntityMesh(const vtkCMBModelEntityMesh&);  // Not implemented.
  void operator=(const vtkCMBModelEntityMesh&);  // Not implemented.
};

#endif

