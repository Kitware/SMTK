//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelFaceMeshOperator - Set the model face mesh parameters.
// .SECTION Description
// Operator to change meshing parameters and initiate meshing on
// the server of a model face.

#ifndef __vtkCMBModelFaceMeshOperator_h
#define __vtkCMBModelFaceMeshOperator_h

#include "cmbSystemConfig.h"
#include <vtkObject.h>

class vtkCMBMeshWrapper;

class VTK_EXPORT vtkCMBModelFaceMeshOperator : public vtkObject
{
public:
  static vtkCMBModelFaceMeshOperator * New();
  vtkTypeMacro(vtkCMBModelFaceMeshOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Modify the color, user name, and/or the visibility of an object.
  virtual void Operate(vtkCMBMeshWrapper* meshWrapper);

  // Description:
  // Set/get the model face's unique persistent Id.
  vtkSetMacro(Id, vtkIdType);
  vtkGetMacro(Id, vtkIdType);

  // Description:
  // Set/get the desired length for the cells discretizing the model face.
  vtkSetClampMacro(Length, double, 0, VTK_FLOAT_MAX);
  vtkGetMacro(Length, double);

  // Description:
  // Set/get the minimum angle for the cells discretizing the model face.
  vtkSetClampMacro(MinimumAngle, double, 0, 60);
  vtkGetMacro(MinimumAngle, double);

  // Description:
  // Set/get whether or not to mesh this model face.
  // 0 means do not and 1 means do.
  vtkSetClampMacro(BuildModelEntityMesh, int, 0, 1);
  vtkGetMacro(BuildModelEntityMesh, int);

  // Description:
  // Set/get whether or not to mesh higher dimensional entities.
  // 0 means do not and 1 means do.
  vtkSetClampMacro(MeshHigherDimensionalEntities, int, 0, 1);
  vtkGetMacro(MeshHigherDimensionalEntities, int);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

  // Description:
  // Returns (1) if the failure was caused by no mesher being found
  // to generate the face
  vtkGetMacro(FaceMesherFailed, int);

protected:
  vtkCMBModelFaceMeshOperator();
  virtual ~vtkCMBModelFaceMeshOperator();

private:
  vtkCMBModelFaceMeshOperator(const vtkCMBModelFaceMeshOperator&);  // Not implemented.
  void operator=(const vtkCMBModelFaceMeshOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  // Description:
  // Flag to describe if the failure of the meshing was casued by no mesher
  // existing or it crashing
  int FaceMesherFailed;

  vtkIdType Id;
  double Length;
  double MinimumAngle;
  int BuildModelEntityMesh;
  int MeshHigherDimensionalEntities;
};

#endif
