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
#include "vtkModelEdge.h"

#include "vtkModel.h"
#include "vtkModelEdgeUse.h"
#include "vtkModelFace.h"
#include "vtkModelItemGenericIterator.h"
#include "vtkModelItemIterator.h"
#include "vtkModelLoopUse.h"
#include "vtkModelVertex.h"
#include "vtkModelVertexUse.h"
#include "vtkObjectFactory.h"

#include <set>


vtkModelEdge::vtkModelEdge()
{
}

//-----------------------------------------------------------------------------
vtkModelEdge::~vtkModelEdge()
{
}

//-----------------------------------------------------------------------------
int vtkModelEdge::GetType()
{
  return vtkModelEdgeType;
}

//-----------------------------------------------------------------------------
int vtkModelEdge::GetNumberOfModelEdgeUses()
{
  return this->GetNumberOfAssociations(vtkModelEdgeUseType);
}

//-----------------------------------------------------------------------------
vtkModelEdgeUse* vtkModelEdge::GetModelEdgeUse(int which)
{ // 0 <= which < number of modeledgeuses
  if(which >= this->GetNumberOfAssociations(vtkModelEdgeUseType) || which < 0)
    {
    return NULL;
    }
  vtkModelItemIterator* iter =
    this->NewIterator(vtkModelEdgeUseType);
  int counter = 0;
  vtkModelEdgeUse* edgeUse = 0;
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    if(counter == which)
      {
      edgeUse = vtkModelEdgeUse::SafeDownCast(iter->GetCurrentItem());
      break;
      }
    counter++;
    }
  iter->Delete();
  return edgeUse;
}

//-----------------------------------------------------------------------------
vtkModelItemIterator* vtkModelEdge::NewModelEdgeUseIterator()
{
  return this->NewIterator(vtkModelEdgeUseType);
}

//-----------------------------------------------------------------------------
int vtkModelEdge::GetNumberOfModelVertexUses()
{
  vtkModelEdgeUse* edgeUse = this->GetModelEdgeUse(0);
  return edgeUse->GetNumberOfAssociations(vtkModelVertexUseType);
}

//-----------------------------------------------------------------------------
vtkModelVertex* vtkModelEdge::GetAdjacentModelVertex(int which)
{
  if(vtkModelVertexUse* vertexUse =
     this->GetModelEdgeUse(1)->GetModelVertexUse(which))
    {
    return vertexUse->GetModelVertex();
    }
  return 0;
}

//-----------------------------------------------------------------------------
void vtkModelEdge::Initialize(vtkModelVertex* vertex0, vtkModelVertex* vertex1,
                              vtkIdType edgeId)
{
  vtkModelEdgeUse* edgeUse0 = vtkModelEdgeUse::New();
  vtkModelEdgeUse* edgeUse1 = vtkModelEdgeUse::New();
  edgeUse0->Initialize(vertex1, vertex0, edgeUse1, 0); //0 is for opposite direction
  edgeUse1->Initialize(vertex0, vertex1, edgeUse0, 1); //1 is for same direction
  this->AddAssociation(edgeUse0);
  edgeUse0->Delete();
  this->AddAssociation(edgeUse1);
  edgeUse1->Delete();
  this->SetUniquePersistentId(edgeId);
}

//-----------------------------------------------------------------------------
vtkModelEdgeUse* vtkModelEdge::BuildModelEdgeUsePair()
{
  if(this->GetNumberOfModelEdgeUses() == 0)
    {
    vtkErrorMacro("Must have an existing edge use before calling BuildModelEdgeUse.");
    return 0;
    }
  vtkModelEdgeUse* firstEdgeUse = this->GetModelEdgeUse(0);

  // first edge use is in opposite direction of edge
  vtkModelVertexUse* vertexUse1 = firstEdgeUse->GetModelVertexUse(0);
  vtkModelVertexUse* vertexUse0 = firstEdgeUse->GetModelVertexUse(1);
  vtkModelVertex *vertex0 = 0, *vertex1 = 0;
  if(vertexUse0)
    {
    vertex0 = vertexUse0->GetModelVertex();
    }
  if(vertexUse1)
    {
    vertex1 = vertexUse1->GetModelVertex();
    }

  vtkModelEdgeUse* edgeUse0 = vtkModelEdgeUse::New();
  vtkModelEdgeUse* edgeUse1 = vtkModelEdgeUse::New();
  edgeUse0->Initialize(vertex1, vertex0, edgeUse1, 0);
  edgeUse1->Initialize(vertex0, vertex1, edgeUse0, 1);
  this->AddAssociation(edgeUse0);
  this->AddAssociation(edgeUse1);
  edgeUse0->Delete();
  edgeUse1->Delete();
  return edgeUse1;
}

//-----------------------------------------------------------------------------
void vtkModelEdge::DestroyModelEdgeUse(vtkModelEdgeUse* edgeUse)
{
  this->RemoveAssociation(edgeUse);
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkModelEdge::SplitModelEdge(
  vtkModelVertex* newVertex, vtkModelEdge* newEdge)
{
  // make sure that we only store a single edge of the paired edges
  // that edge use is in the same direction as the model edge (dir=1)
  std::set<vtkModelEdgeUse*> edgeUsePairs;

  vtkModelItemIterator* edgeUses = this->NewIterator(vtkModelEdgeUseType);
  for(edgeUses->Begin();!edgeUses->IsAtEnd();edgeUses->Next())
    {
    vtkModelEdgeUse* edgeUse = vtkModelEdgeUse::SafeDownCast(
      edgeUses->GetCurrentItem());
    if(edgeUse->GetDirection() == 1)
      {
      edgeUsePairs.insert(edgeUse);
      }
    else
      {
      edgeUsePairs.insert(edgeUse->GetPairedModelEdgeUse());
      }
    }
  edgeUses->Delete();

  vtkModelVertex* originalVertex0 = this->GetAdjacentModelVertex(0);
  vtkModelVertex* originalVertex1 = this->GetAdjacentModelVertex(1);

  // we already have an existing edge use pair for the first pass
  std::set<vtkModelEdgeUse*>::iterator it=edgeUsePairs.begin();
  vtkModelEdgeUse* newEdgeUse = newEdge->GetModelEdgeUse(1);

  vtkModelVertexUse* originalVertexUse0 = (*it)->GetModelVertexUse(0);
  vtkModelVertexUse* originalVertexUse1 = (*it)->GetModelVertexUse(1);
  vtkModelVertexUse* vertexUse = newVertex->BuildModelVertexUse();
  this->SplitModelEdgeUse(*it, newEdgeUse, originalVertexUse0,
                          vertexUse, originalVertexUse1);
  int index = (*it)->GetModelLoopUse()->GetModelEdgeUseIndex(*it);
  (*it)->GetModelLoopUse()->InsertModelEdgeUse(index+1, newEdgeUse);

  originalVertexUse0 = (*it)->GetPairedModelEdgeUse()->GetModelVertexUse(0);
  originalVertexUse1 = (*it)->GetPairedModelEdgeUse()->GetModelVertexUse(1);
  vertexUse = newVertex->BuildModelVertexUse();
  vtkModelEdgeUse* oldPairedModelEdgeUse = (*it)->GetPairedModelEdgeUse();
  this->SplitModelEdgeUse(
    newEdgeUse->GetPairedModelEdgeUse(), oldPairedModelEdgeUse,
    originalVertexUse0, vertexUse, originalVertexUse1);
  index = oldPairedModelEdgeUse->GetModelLoopUse()
    ->GetModelEdgeUseIndex(oldPairedModelEdgeUse);
  oldPairedModelEdgeUse->GetModelLoopUse()->InsertModelEdgeUse(
    index, newEdgeUse->GetPairedModelEdgeUse());

  for(it++ ;it!=edgeUsePairs.end();it++)
    {
    newEdgeUse = newEdge->BuildModelEdgeUsePair();

    // BuildeModelEdgeUsePair creates model vertex uses that we don't
    // want so we remove them now
    vtkModelVertexUse* vertexUse0 = newEdgeUse->GetModelVertexUse(0);
    vtkModelVertexUse* vertexUse1 = newEdgeUse->GetModelVertexUse(1);
    newEdgeUse->SetModelVertexUses(0, 0);
    if(vertexUse0 && vertexUse0->GetNumberOfAssociations(vtkModelEdgeUseType) == 0)
      {
      vertexUse0->GetModelVertex()->DestroyModelVertexUse(vertexUse0);
      }
    if(vertexUse1 && vertexUse1 != vertexUse0 &&
       vertexUse1->GetNumberOfAssociations(vtkModelEdgeUseType) == 0)
      {
      vertexUse1->GetModelVertex()->DestroyModelVertexUse(vertexUse1);
      }

    vertexUse0 = newEdgeUse->GetPairedModelEdgeUse()->GetModelVertexUse(0);
    vertexUse1 = newEdgeUse->GetPairedModelEdgeUse()->GetModelVertexUse(1);
    newEdgeUse->GetPairedModelEdgeUse()->SetModelVertexUses(0, 0);
    if(vertexUse0 && vertexUse0->GetNumberOfAssociations(vtkModelEdgeUseType) == 0)
      {
      vertexUse0->GetModelVertex()->DestroyModelVertexUse(vertexUse0);
      }
    if(vertexUse1 && vertexUse1 != vertexUse0 &&
       vertexUse1->GetNumberOfAssociations(vtkModelEdgeUseType) == 0)
      {
      vertexUse1->GetModelVertex()->DestroyModelVertexUse(vertexUse1);
      }

    originalVertexUse0 = (*it)->GetModelVertexUse(0);
    originalVertexUse1 = (*it)->GetModelVertexUse(1);
    vertexUse = newVertex->BuildModelVertexUse();
    this->SplitModelEdgeUse(*it, newEdgeUse, originalVertexUse0,
                            vertexUse, originalVertexUse1);

    index = (*it)->GetModelLoopUse()->GetModelEdgeUseIndex(*it);
    (*it)->GetModelLoopUse()->InsertModelEdgeUse(index+1, newEdgeUse);

    originalVertexUse0 = (*it)->GetPairedModelEdgeUse()->GetModelVertexUse(0);
    originalVertexUse1 = (*it)->GetPairedModelEdgeUse()->GetModelVertexUse(1);
    vertexUse = newVertex->BuildModelVertexUse();
    this->SplitModelEdgeUse(
      newEdgeUse->GetPairedModelEdgeUse(), (*it)->GetPairedModelEdgeUse(),
      originalVertexUse0, vertexUse, originalVertexUse1);

    oldPairedModelEdgeUse = (*it)->GetPairedModelEdgeUse();
    index = oldPairedModelEdgeUse->GetModelLoopUse()
      ->GetModelEdgeUseIndex(oldPairedModelEdgeUse);
    oldPairedModelEdgeUse->GetModelLoopUse()->InsertModelEdgeUse(
      index, newEdgeUse->GetPairedModelEdgeUse());
    }
}

//-----------------------------------------------------------------------------
void vtkModelEdge::SplitModelEdgeUse(
  vtkModelEdgeUse* firstEdgeUse, vtkModelEdgeUse* secondEdgeUse,
  vtkModelVertexUse* vertexUse0, vtkModelVertexUse* vertexUse1,
  vtkModelVertexUse* vertexUse2)
{
  firstEdgeUse->SetModelVertexUses(vertexUse0, vertexUse1);
  secondEdgeUse->SetModelVertexUses(vertexUse1, vertexUse2);
}

//-----------------------------------------------------------------------------
bool vtkModelEdge::SplitModelEdgeLoop(vtkModelVertex* vertex)
{
  vtkModelItemIterator* edgeUses = this->NewIterator(vtkModelEdgeUseType);
  for(edgeUses->Begin();!edgeUses->IsAtEnd();edgeUses->Next())
    {
    vtkModelVertexUse* vertexUse = vertex->BuildModelVertexUse();
    vtkModelEdgeUse* edgeUse =
      vtkModelEdgeUse::SafeDownCast(edgeUses->GetCurrentItem());
    edgeUse->SetModelVertexUses(vertexUse, vertexUse);
    }
  edgeUses->Delete();
  this->GetModel()->InvokeModelGeometricEntityEvent(
    ModelGeometricEntityBoundaryModified, this);
  return 1;
}

//-----------------------------------------------------------------------------
int vtkModelEdge::GetNumberOfAdjacentModelFaces()
{
  std::set<vtkModelFace*> faces;
  vtkModelItemIterator* edgeUses =
    this->NewIterator(vtkModelEdgeUseType);
  for(edgeUses->Begin();!edgeUses->IsAtEnd();edgeUses->Next())
    {
    vtkModelItemIterator* loopUses =
      edgeUses->GetCurrentItem()->NewIterator(vtkModelLoopUseType);
    for(loopUses->Begin();!loopUses->IsAtEnd();loopUses->Next())
      {
      faces.insert(vtkModelLoopUse::SafeDownCast(loopUses->GetCurrentItem())->
                   GetModelFace());
      }
    loopUses->Delete();
    }
  edgeUses->Delete();

  return static_cast<int>(faces.size());
}

//-----------------------------------------------------------------------------
vtkModelItemIterator* vtkModelEdge::NewAdjacentModelFaceIterator()
{
  vtkModelItemGenericIterator* faces = vtkModelItemGenericIterator::New();
  vtkModelItemIterator* edgeUses =
    this->NewIterator(vtkModelEdgeUseType);
  for(edgeUses->Begin();!edgeUses->IsAtEnd();edgeUses->Next())
    {
    vtkModelItemIterator* loopUses =
      edgeUses->GetCurrentItem()->NewIterator(vtkModelLoopUseType);
    for(loopUses->Begin();!loopUses->IsAtEnd();loopUses->Next())
      {
      faces->AddUniqueModelItem(
        vtkModelLoopUse::SafeDownCast(loopUses->GetCurrentItem())->
        GetModelFace());
      }
    loopUses->Delete();
    }
  edgeUses->Delete();

  return faces;
}

//-----------------------------------------------------------------------------
void vtkModelEdge::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

//-----------------------------------------------------------------------------
void vtkModelEdge::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

