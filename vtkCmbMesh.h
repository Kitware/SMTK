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
// .NAME vtkCmbMesh - Mesh representation for a vtkModel.
// .SECTION Description

#ifndef __vtkCmbMesh_h
#define __vtkCmbMesh_h

#include <vtkObject.h>

class vtkCmbMeshInternals;
class vtkModel;
class vtkCmbModelEntityMesh;
class vtkModelGeometricEntity;

class VTK_EXPORT vtkCmbMesh : public vtkObject
{
public:
  static vtkCmbMesh* New();
  vtkTypeRevisionMacro(vtkCmbMesh,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get visible (non-zero is visible).
  vtkSetMacro(Visible, bool);
  vtkGetMacro(Visible, bool);

  void Initialize(vtkModel* model);

  // Description:
  // If GlobalLength is less than or equal to zero, it
  // is still uninitialized.
  void SetGlobalLength(double length);
  vtkGetMacro(GlobalLength, double);

  void Reset();

  // Description:
  // Given a vtkModelGeometricEntity, get the associated mesh
  // representation.
  vtkCmbModelEntityMesh* GetModelEntityMesh(vtkModelGeometricEntity*);

protected:
  vtkCmbMesh();
  virtual ~vtkCmbMesh();

  static void ModelGeometricEntityChanged(
    vtkObject *caller, unsigned long event,
    void *cData, void *callData);

private:
  vtkCmbMesh(const vtkCmbMesh&);  // Not implemented.
  void operator=(const vtkCmbMesh&);  // Not implemented.

  bool Visible;
  double GlobalLength;
  vtkCmbMeshInternals* Internal;
};

#endif

