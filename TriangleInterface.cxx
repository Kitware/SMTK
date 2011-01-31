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
// .NAME vtkCmbModelFaceMeshPrivate
// .SECTION Description
// Convert a vtkModelFace to a triangle input for meshing. Also
// restores the resulting mesh to a vtkPolyData

#include "TriangleInterface.h"

#include <vtkstd/map> // Needed for STL map.
#include <vtkstd/set> // Needed for STL set.
#include <vtkstd/list> // Needed for STL list.

#include <vtkPolyData.h>

extern "C"
{
#include "triangle.h"
#include "share_extern.h"
  void Init_triangluateio(struct triangulateio *);
  void Free_triangluateio(struct triangulateio *);
}
// END for Triangle

struct TriangleInterface::TriangleIO
  {
  triangulateio *in;
  triangulateio *out;
  };

//----------------------------------------------------------------------------
TriangleInterface::TriangleInterface( MeshInformation* info):
  MaxArea(-1),
  MinAngle(-1),
  OutputMesh(NULL),
  TriIO(NULL);
{
  this->MeshInfo = info;
  this->InitDataStructures();
}

//----------------------------------------------------------------------------
void TriangleInterface::InitDataStructures()
{
  this->TriIO->in = (triangulateio*)malloc(sizeof(triangulateio));
  this->TriIO->out = (triangulateio*)malloc(sizeof(triangulateio));
  Init_triangluateio(this->TriIO->in);
  Init_triangluateio(this->TriIO->out);
}

//----------------------------------------------------------------------------
void TriangleInterface::setOutputMesh(vtkPolyData *mesh)
{
  if ( this->OutputMesh )
    {
    this->OutputMesh->UnRegister(this);
    }
  this->OutputMesh->ShallowCopy(mesh);
}

//----------------------------------------------------------------------------
bool TriangleInterface::buildFaceMesh()
{

  return true;
}