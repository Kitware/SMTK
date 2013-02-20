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
#include "vtkModelVertexUse.h"

#include "vtkModel.h"
#include "vtkModelEdgeUse.h"
#include "vtkModelItemIterator.h"
#include "vtkModelVertex.h"
#include "vtkObjectFactory.h"


vtkModelVertexUse* vtkModelVertexUse::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkModelVertexUse"); 
  if(ret) 
    {                                    
    return static_cast<vtkModelVertexUse*>(ret); 
    } 
  return new vtkModelVertexUse;
}

vtkModelVertexUse::vtkModelVertexUse()
{
}

vtkModelVertexUse::~vtkModelVertexUse()
{
}

bool vtkModelVertexUse::Destroy()
{
  // the model edge use calls destroy on the vertex use
  // and maintains a reference to it
  this->RemoveAllAssociations(vtkModelVertexType);
  return true;
}

int vtkModelVertexUse::GetType()
{
  return vtkModelVertexUseType;
}

vtkModelVertex* vtkModelVertexUse::GetModelVertex()
{
  vtkModelItemIterator* iter = 
    this->NewIterator(vtkModelVertexType);
  iter->Begin();
  vtkModelVertex* Vertex = 
    vtkModelVertex::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return Vertex;
}

int vtkModelVertexUse::GetNumberOfModelEdgeUses()
{
  return this->GetNumberOfAssociations(vtkModelEdgeUseType);
}

vtkModelItemIterator* vtkModelVertexUse::NewModelEdgeUseIterator()
{
  return this->NewIterator(vtkModelEdgeUseType);
}

void vtkModelVertexUse::Initialize(vtkModelVertex* Vertex)
{
  this->AddAssociation(Vertex->GetType(), Vertex);
}

void vtkModelVertexUse::AddModelEdgeUse(vtkModelEdgeUse* EdgeUse)
{
  this->AddAssociation(EdgeUse->GetType(), EdgeUse);
}

void vtkModelVertexUse::RemoveModelEdgeUse(vtkModelEdgeUse* EdgeUse)
{
  this->RemoveAssociation(EdgeUse->GetType(), EdgeUse);
}

void vtkModelVertexUse::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

void vtkModelVertexUse::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

