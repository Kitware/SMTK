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
#include "vtkModelEdgeUse.h"

#include "vtkInformation.h"
#include "vtkInformationIntegerKey.h"
#include "vtkModel.h"
#include "vtkModelEdge.h"
#include "vtkModelItemIterator.h"
#include "vtkModelLoopUse.h"
#include "vtkModelVertex.h"
#include "vtkModelVertexUse.h"
#include "vtkObjectFactory.h"

vtkInformationKeyMacro(vtkModelEdgeUse, DIRECTION, Integer);

vtkModelEdgeUse* vtkModelEdgeUse::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkModelEdgeUse"); 
  if(ret) 
    {                                    
    return static_cast<vtkModelEdgeUse*>(ret); 
    } 
  return new vtkModelEdgeUse;
}

vtkModelEdgeUse::vtkModelEdgeUse()
{
}

vtkModelEdgeUse::~vtkModelEdgeUse()
{
}

bool vtkModelEdgeUse::Destroy()
{
  if(this->GetModelLoopUse())
    {
    vtkErrorMacro("Trying to remove a ModelEdgeUse that is still connected to a ModelLoopUse.");
    return false;
    }
  // if I'm connected to a vertex use that isn't connected to any other edge uses
  // then I need to destroy that vertex use too.  

  vtkModelVertexUse* VertexUse0 = this->GetModelVertexUse(0);
  if(VertexUse0)
    {
    if(VertexUse0->GetNumberOfModelEdgeUses() == 1)
      {
      VertexUse0->Destroy();
      }
    }
  vtkModelVertexUse* VertexUse1 = this->GetModelVertexUse(1);
  if(VertexUse1)
    {
    if(VertexUse1->GetNumberOfModelEdgeUses() == 1)
      {
      VertexUse1->Destroy();
      }
    else if(VertexUse0 == VertexUse1)
      {
      // also if this edge use is a loop then I'm connected to the same 
      // vertex use twice and must also remove both uses
      VertexUse0->Destroy();
      }
    }

  this->RemoveAllAssociations(vtkModelVertexUseType);
  // my pair will have to remove me separately
  this->RemoveReverseAssociationToType(this->GetPairedModelEdgeUse(),this->GetType());
  
  return true;
}

int vtkModelEdgeUse::GetType()
{
  return vtkModelEdgeUseType;
}

vtkModelEdge* vtkModelEdgeUse::GetModelEdge()
{
  vtkModelItemIterator* iter = 
    this->NewIterator(vtkModelEdgeType);
  iter->Begin();
  vtkModelEdge* Edge = vtkModelEdge::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return Edge;
}

vtkModelEdgeUse* vtkModelEdgeUse::GetPairedModelEdgeUse()
{
  vtkModelItemIterator* iter = 
    this->NewIterator(vtkModelEdgeUseType);
  iter->Begin();
  vtkModelEdgeUse* EdgeUse = vtkModelEdgeUse::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return EdgeUse;
}

vtkModelLoopUse* vtkModelEdgeUse::GetModelLoopUse()
{
  vtkModelItemIterator* iter = 
    this->NewIterator(vtkModelLoopUseType);
  iter->Begin();
  vtkModelLoopUse* LoopUse = vtkModelLoopUse::SafeDownCast(
    iter->GetCurrentItem());
  iter->Delete();
  return LoopUse;
}

void vtkModelEdgeUse::SetModelVertexUses(vtkModelVertexUse* VertexUse0, 
                                         vtkModelVertexUse* VertexUse1)
{
  this->RemoveAllAssociations(vtkModelVertexUseType);
  if(VertexUse0)
    {
    this->AddAssociation(VertexUse0);
    if(VertexUse1)
      {
      this->AddAssociation(VertexUse1);
      }
    }
  else if(VertexUse1)
    {
    vtkWarningMacro("Trying to set VertexUse1 without setting VertexUse0");
    }
}

vtkModelVertexUse* vtkModelEdgeUse::GetModelVertexUse(int i)
{
  vtkModelItemIterator* iter = this->NewIterator(vtkModelVertexUseType);
  iter->Begin();
  if(iter->IsAtEnd())
    {
    iter->Delete();
    return 0;
    }
  if(i == 0)
    {
    vtkModelVertexUse* VertexUse = 
      vtkModelVertexUse::SafeDownCast(iter->GetCurrentItem());
    iter->Delete();
    return VertexUse;
    }
  iter->Next();
  if(iter->IsAtEnd())
    {
    iter->Delete();
    return 0;
    }
  vtkModelVertexUse* VertexUse = 
    vtkModelVertexUse::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return VertexUse;  
}

int vtkModelEdgeUse::GetNumberOfModelVertexUses()
{
  return this->GetNumberOfAssociations(vtkModelVertexUseType);
}

int vtkModelEdgeUse::GetDirection()
{
  return this->GetProperties()->Get(DIRECTION());
}

void vtkModelEdgeUse::Initialize(
  vtkModelVertex* Vertex0, vtkModelVertex* Vertex1,
  vtkModelEdgeUse* PairedEdgeUse, int direction)
{
  if(Vertex0)
    {
    vtkModelVertexUse* VertexUse = vtkModelVertexUse::New();
    VertexUse->Initialize(Vertex0);
    this->AddAssociation(VertexUse);
    VertexUse->Delete();
    if(Vertex1)
      {
      VertexUse = vtkModelVertexUse::New();
      VertexUse->Initialize(Vertex1);
      this->AddAssociation(VertexUse);
      VertexUse->Delete();
      }
    }
  if(direction != 0)
    {
    direction = 1;
    }
  this->SetDirection(direction);
  // only add reverse association as my pair also will have to call
  // initialize and do the same thing
  this->AddReverseAssociationToType(PairedEdgeUse,vtkModelEdgeUseType);
}


void vtkModelEdgeUse::SetDirection(int direction)
{
  this->GetProperties()->Set(DIRECTION(), direction);
}

void vtkModelEdgeUse::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

void vtkModelEdgeUse::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

