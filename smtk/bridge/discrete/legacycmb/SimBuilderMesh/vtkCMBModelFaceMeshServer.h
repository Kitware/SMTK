//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelFaceMeshServer - Mesh representation for a vtkModelFace
// .SECTION Description
// Mesh representation for a vtkModelFace.  The smaller value for
// maximum area and larger value for minimum angle wins for local
// vs. global comparisons.  Absolute values are used for area.

#ifndef __vtkCMBModelFaceMeshServer_h
#define __vtkCMBModelFaceMeshServer_h

#include "vtkCMBModelFaceMesh.h"
#include "cmbSystemConfig.h"

class vtkModelFace;
class vtkCMBModelVertexMesh;

class VTK_EXPORT vtkCMBModelFaceMeshServer : public vtkCMBModelFaceMesh
{
public:
  static vtkCMBModelFaceMeshServer* New();
  vtkTypeMacro(vtkCMBModelFaceMeshServer,vtkCMBModelFaceMesh);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the local mesh length, min angle
  virtual bool SetLocalLength(double length);
  virtual bool SetLocalMinimumAngle(double angle);

  int GetFaceMesherFailed() const { return this->FaceMesherFailed; }

protected:
  vtkCMBModelFaceMeshServer();
  virtual ~vtkCMBModelFaceMeshServer();

  // Description:
  // This method builds the model entity's mesh without checking
  // the parameters.  Returns true if the operation succeeded as
  // desired.  This includes deleting the mesh if the mesh
  // parameters go from valid to invalid values (i.e. a parameter set to 0).
  bool BuildMesh(bool meshHigherDimensionalEntities);

  bool CreateMeshInfo();
  bool Triangulate(vtkPolyData *mesh, double length, double angle);
  void DetermineZValueOfFace();

private:
  int FaceMesherFailed;

  double ZValue;
  vtkCMBModelFaceMeshServer(const vtkCMBModelFaceMeshServer&);  // Not implemented.
  void operator=(const vtkCMBModelFaceMeshServer&);  // Not implemented.

  CmbFaceMesherClasses::ModelFaceRep *FaceInfo;

};

#endif
