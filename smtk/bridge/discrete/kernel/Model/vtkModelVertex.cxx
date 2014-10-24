//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelVertex.h"

#include "vtkInformation.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkModel.h"
#include "vtkModelEdge.h"
#include "vtkModelEdgeUse.h"
#include "vtkModelItemGenericIterator.h"
#include "vtkModelVertexUse.h"
#include "vtkObjectFactory.h"

#include <map>


vtkModelVertex::vtkModelVertex()
{
}

vtkModelVertex::~vtkModelVertex()
{
}

bool vtkModelVertex::IsDestroyable()
{
  if(this->GetNumberOfAssociations(vtkModelVertexUseType) != 0)
    {
    return 0;
    }
  return 1;
}

bool vtkModelVertex::Destroy()
{
  this->GetModel()->InvokeModelGeometricEntityEvent(
    ModelGeometricEntityAboutToDestroy, this);
  return 1;
}

int vtkModelVertex::GetType()
{
  return vtkModelVertexType;
}

bool vtkModelVertex::GetBounds(double bounds[6])
{
  double xyz[3];
  if(this->GetPoint(xyz))
    {
    for(int i=0;i<3;i++)
      {
      bounds[2*i+1] = bounds[2*i] = xyz[i];
      }
    return true;
    }
  return false;
}

void vtkModelVertex::DestroyModelVertexUse(vtkModelVertexUse* vertexUse)
{
  this->RemoveAssociation(vertexUse);
}

int vtkModelVertex::GetNumberOfModelVertexUses()
{
  return this->GetNumberOfAssociations(vtkModelVertexUseType);
}

vtkModelItemIterator* vtkModelVertex::NewModelVertexUseIterator()
{
  vtkModelItemIterator* iter =
    this->NewIterator(vtkModelVertexUseType);
  return iter;
}

vtkModelItemIterator* vtkModelVertex::NewAdjacentModelEdgeIterator()
{
  std::map<vtkIdType, vtkModelEdge*> modelEdges;
  vtkModelItemIterator* vertexUses =
    this->NewIterator(vtkModelVertexUseType);
  for(vertexUses->Begin();!vertexUses->IsAtEnd();vertexUses->Next())
    {
    vtkModelVertexUse* vertexUse =
      vtkModelVertexUse::SafeDownCast(vertexUses->GetCurrentItem());
    vtkModelItemIterator* edgeUses = vertexUse->NewModelEdgeUseIterator();
    for(edgeUses->Begin();!edgeUses->IsAtEnd();edgeUses->Next())
      {
      vtkModelEdgeUse* edgeUse =
        vtkModelEdgeUse::SafeDownCast(edgeUses->GetCurrentItem());
      vtkModelEdge* edge = edgeUse->GetModelEdge();
      modelEdges[edge->GetUniquePersistentId()] = edge;
      }
    edgeUses->Delete();
    }
  vertexUses->Delete();
  vtkModelItemGenericIterator* edges = vtkModelItemGenericIterator::New();
  for(std::map<vtkIdType, vtkModelEdge*>::iterator it=modelEdges.begin();
      it!=modelEdges.end();it++)
    {
    edges->AddModelItem(it->second);
    }
  return edges;
}

vtkModelVertexUse* vtkModelVertex::BuildModelVertexUse()
{
  vtkModelVertexUse* VertexUse = vtkModelVertexUse::New();
  this->AddAssociation(VertexUse);
  VertexUse->Delete();
  return VertexUse;
}

void vtkModelVertex::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

void vtkModelVertex::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
