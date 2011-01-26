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
// .NAME vtkCmbModelEntityMesh - Mesh representation for a vtkModelGeometricEntity
// .SECTION Description
// Class is abstract.

#ifndef __vtkCmbModelEntityMesh_h
#define __vtkCmbModelEntityMesh_h

#include <vtkObject.h>

#include <vtkModelGeometricEntity.h>
#include <vtkWeakPointer.h>

class vtkPolyData;
class vtkCmbMesh;

class VTK_EXPORT vtkCmbModelEntityMesh : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkCmbModelEntityMesh,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get visible (true is visible).
  vtkSetMacro(Visible, bool);
  vtkGetMacro(Visible, bool);

  virtual vtkModelGeometricEntity* GetModelGeometricEntity() = 0;

  vtkGetMacro(MasterMesh, vtkCmbMesh*);

  vtkGetMacro(ModelEntityMesh, vtkPolyData*);

  virtual bool BuildModelEntityMesh() = 0;

protected:
  vtkCmbModelEntityMesh();
  virtual ~vtkCmbModelEntityMesh();

  // Description:
  // Mesh is reference counted.
  void SetModelEntityMesh(vtkPolyData* mesh);

  // Description:
  // Mesh is not reference counted.
  vtkSetMacro(MasterMesh, vtkCmbMesh*);

private:
  bool Visible;
  vtkCmbMesh* MasterMesh;
  vtkPolyData* ModelEntityMesh;

  vtkCmbModelEntityMesh(const vtkCmbModelEntityMesh&);  // Not implemented.
  void operator=(const vtkCmbModelEntityMesh&);  // Not implemented.
};

#endif

