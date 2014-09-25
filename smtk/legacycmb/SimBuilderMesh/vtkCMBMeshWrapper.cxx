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
#include "vtkCMBMeshWrapper.h"

#include <vtkObjectFactory.h>
#include "vtkCMBMeshServer.h"
#include <vtkDiscreteModel.h>
#include <vtkDiscreteModelWrapper.h>

#include <vtksys/ios/sstream>

vtkStandardNewMacro(vtkCMBMeshWrapper);

//----------------------------------------------------------------------------
vtkCMBMeshWrapper::vtkCMBMeshWrapper()
{
  this->Mesh = NULL;
}

//----------------------------------------------------------------------------
vtkCMBMeshWrapper::~vtkCMBMeshWrapper()
{
  if(this->Mesh)
    {
    this->Mesh->Delete();
    this->Mesh = NULL;
    }
}

//----------------------------------------------------------------------------
vtkCMBMeshServer* vtkCMBMeshWrapper::GetMesh()
{
  if(!this->Mesh)
    {
    this->Mesh = vtkCMBMeshServer::New();
    }
  return this->Mesh;
}

//----------------------------------------------------------------------------
void vtkCMBMeshWrapper::SetModelWrapper(vtkDiscreteModelWrapper* modelWrapper)
{
  if(modelWrapper == NULL)
    {
    vtkErrorMacro("ModelWrapper is NULL");
    return;
    }
  this->GetMesh()->Initialize(modelWrapper->GetModel());
}

//----------------------------------------------------------------------------
void vtkCMBMeshWrapper::SetGlobalLength(double length)
{
  this->GetMesh()->SetGlobalLength(length);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCMBMeshWrapper::SetGlobalMinimumAngle(double angle)
{
  this->GetMesh()->SetGlobalMinimumAngle(angle);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCMBMeshWrapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  if(this->Mesh)
    {
    os << indent << "Mesh: " << this->Mesh << endl;
    }
  else
    {
    os << indent << "Mesh: (NULL)\n";
    }
}
