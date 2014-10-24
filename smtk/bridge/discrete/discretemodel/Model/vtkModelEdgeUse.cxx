//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

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

  vtkModelVertexUse* vertexUse0 = this->GetModelVertexUse(0);
  if(vertexUse0)
    {
    if(vertexUse0->GetNumberOfModelEdgeUses() == 1)
      {
      vertexUse0->Destroy();
      }
    }
  vtkModelVertexUse* vertexUse1 = this->GetModelVertexUse(1);
  if(vertexUse1)
    {
    if(vertexUse1->GetNumberOfModelEdgeUses() == 1)
      {
      vertexUse1->Destroy();
      }
    else if(vertexUse0 == vertexUse1)
      {
      // also if this edge use is a loop then I'm connected to the same
      // vertex use twice and must also remove both uses
      vertexUse0->Destroy();
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
  vtkModelEdge* edge = vtkModelEdge::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return edge;
}

vtkModelEdgeUse* vtkModelEdgeUse::GetPairedModelEdgeUse()
{
  vtkModelItemIterator* iter =
    this->NewIterator(vtkModelEdgeUseType);
  iter->Begin();
  vtkModelEdgeUse* edgeUse = vtkModelEdgeUse::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return edgeUse;
}

vtkModelLoopUse* vtkModelEdgeUse::GetModelLoopUse()
{
  vtkModelItemIterator* iter =
    this->NewIterator(vtkModelLoopUseType);
  iter->Begin();
  vtkModelLoopUse* loopUse = vtkModelLoopUse::SafeDownCast(
    iter->GetCurrentItem());
  iter->Delete();
  return loopUse;
}

void vtkModelEdgeUse::SetModelVertexUses(vtkModelVertexUse* vertexUse0,
                                         vtkModelVertexUse* vertexUse1)
{
  this->RemoveAllAssociations(vtkModelVertexUseType);
  if(vertexUse0)
    {
    this->AddAssociation(vertexUse0);
    if(vertexUse1)
      {
      this->AddAssociation(vertexUse1);
      }
    }
  else if(vertexUse1)
    {
    vtkWarningMacro("Trying to set vertexUse1 without setting vertexUse0");
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
    vtkModelVertexUse* vertexUse =
      vtkModelVertexUse::SafeDownCast(iter->GetCurrentItem());
    iter->Delete();
    return vertexUse;
    }
  iter->Next();
  if(iter->IsAtEnd())
    {
    iter->Delete();
    return 0;
    }
  vtkModelVertexUse* vertexUse =
    vtkModelVertexUse::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return vertexUse;
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
  vtkModelVertex* vertex0, vtkModelVertex* vertex1,
  vtkModelEdgeUse* pairedEdgeUse, int direction)
{
  if(vertex0)
    {
    vtkModelVertexUse* VertexUse = vtkModelVertexUse::New();
    VertexUse->Initialize(vertex0);
    this->AddAssociation(VertexUse);
    VertexUse->Delete();
    if(vertex1)
      {
      VertexUse = vtkModelVertexUse::New();
      VertexUse->Initialize(vertex1);
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
  this->AddReverseAssociationToType(pairedEdgeUse,vtkModelEdgeUseType);
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
