//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelVertexUse.h"

#include "vtkModel.h"
#include "vtkModelEdgeUse.h"
#include "vtkModelItemIterator.h"
#include "vtkModelVertex.h"
#include "vtkObjectFactory.h"

vtkModelVertexUse* vtkModelVertexUse::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkModelVertexUse");
  if (ret)
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
  vtkModelItemIterator* iter = this->NewIterator(vtkModelVertexType);
  iter->Begin();
  vtkModelVertex* vertex = vtkModelVertex::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return vertex;
}

int vtkModelVertexUse::GetNumberOfModelEdgeUses()
{
  return this->GetNumberOfAssociations(vtkModelEdgeUseType);
}

vtkModelItemIterator* vtkModelVertexUse::NewModelEdgeUseIterator()
{
  return this->NewIterator(vtkModelEdgeUseType);
}

void vtkModelVertexUse::Initialize(vtkModelVertex* vertex)
{
  this->AddAssociation(vertex);
}

void vtkModelVertexUse::AddModelEdgeUse(vtkModelEdgeUse* edgeUse)
{
  this->AddAssociation(edgeUse);
}

void vtkModelVertexUse::RemoveModelEdgeUse(vtkModelEdgeUse* edgeUse)
{
  this->RemoveAssociation(edgeUse);
}

void vtkModelVertexUse::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

void vtkModelVertexUse::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
