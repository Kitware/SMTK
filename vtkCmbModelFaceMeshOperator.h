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
// .NAME vtkCmbModelFaceMeshOperator - Set the model face mesh parameters.
// .SECTION Description
// Operator to change meshing parameters and initiate meshing on
// the server of a model face.

#ifndef __vtkCmbModelFaceMeshOperator_h
#define __vtkCmbModelFaceMeshOperator_h

#include <vtkObject.h>

class vtkCmbMeshWrapper;

class VTK_EXPORT vtkCmbModelFaceMeshOperator : public vtkObject
{
public:
  static vtkCmbModelFaceMeshOperator * New();
  vtkTypeMacro(vtkCmbModelFaceMeshOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Modify the color, user name, and/or the visibility of an object.
  virtual void Operate(vtkCmbMeshWrapper* meshWrapper);

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
  vtkCmbModelFaceMeshOperator();
  virtual ~vtkCmbModelFaceMeshOperator();

private:
  vtkCmbModelFaceMeshOperator(const vtkCmbModelFaceMeshOperator&);  // Not implemented.
  void operator=(const vtkCmbModelFaceMeshOperator&);  // Not implemented.

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
