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

void vtkModelVertex::DestroyModelVertexUse(vtkModelVertexUse* VertexUse)
{
  this->RemoveAssociation(VertexUse->GetType(), VertexUse);
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
