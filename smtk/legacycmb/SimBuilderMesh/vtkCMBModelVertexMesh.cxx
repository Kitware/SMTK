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
#include "vtkCMBModelVertexMesh.h"

#include <vtkObjectFactory.h>
#include <vtkModelVertex.h>

vtkStandardNewMacro(vtkCMBModelVertexMesh);

//----------------------------------------------------------------------------
vtkCMBModelVertexMesh::vtkCMBModelVertexMesh()
{
  this->ModelVertex = NULL;
}

//----------------------------------------------------------------------------
vtkCMBModelVertexMesh::~vtkCMBModelVertexMesh()
{
}

//----------------------------------------------------------------------------
vtkModelGeometricEntity* vtkCMBModelVertexMesh::GetModelGeometricEntity()
{
  return this->ModelVertex;
}

//----------------------------------------------------------------------------
void vtkCMBModelVertexMesh::Initialize(vtkCMBMesh* masterMesh,
                                       vtkModelVertex* vertex)
{
  if(this->GetMasterMesh() != masterMesh)
    {
    this->SetMasterMesh(masterMesh);
    this->Modified();
    }
  if(this->ModelVertex != vertex)
    {
    this->ModelVertex = vertex;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
bool vtkCMBModelVertexMesh::BuildModelEntityMesh(
  bool vtkNotUsed(meshHigherDimensionalEntities) )
{
  return false;
}

//----------------------------------------------------------------------------
void vtkCMBModelVertexMesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  if(this->ModelVertex)
    {
    os << indent << "ModelVertex: " << this->ModelVertex << "\n";
    }
  else
    {
    os << indent << "ModelVertex: (NULL)\n";
    }
}

