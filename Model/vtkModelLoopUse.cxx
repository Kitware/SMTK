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
#include "vtkModelLoopUse.h"

#include "vtkModel.h"
#include "vtkModelEdge.h"
#include "vtkModelEdgeUse.h"
#include "vtkModelFace.h"
#include "vtkModelFaceUse.h"
#include "vtkModelItemIterator.h"
#include "vtkModelVertex.h"
#include "vtkModelVertexUse.h"
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

#include <set>


vtkModelLoopUse* vtkModelLoopUse::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkModelLoopUse");
  if(ret)
    {
    return static_cast<vtkModelLoopUse*>(ret);
    }
  return new vtkModelLoopUse;
}

vtkModelLoopUse::vtkModelLoopUse()
{
}

vtkModelLoopUse::~vtkModelLoopUse()
{
}

bool vtkModelLoopUse::Destroy()
{
  // a modelloopuse shouldn't be connected to any model use object of higher dimension
  // Here we go through each model edge use of this loopuse and destroy any
  // endge use whose edge contains more than 2 edge uses which are by default for an edge.
  vtkSmartPointer<vtkModelItemIterator> edgeUses;
  edgeUses.TakeReference(this->NewIterator(vtkModelEdgeUseType));
  std::set<vtkModelEdgeUse*> destroyEdgeUses;
  for(edgeUses->Begin();!edgeUses->IsAtEnd();edgeUses->Next())
    {
    vtkModelEdgeUse* edgeUse = vtkModelEdgeUse::SafeDownCast(edgeUses->GetCurrentItem());
    if(edgeUse->GetModelEdge()->GetNumberOfModelEdgeUses() > 2)
      {
     destroyEdgeUses.insert(edgeUse);
      }
    }

  this->RemoveAllAssociations(vtkModelEdgeUseType);
  for(std::set<vtkModelEdgeUse*>::iterator it=destroyEdgeUses.begin();
      it != destroyEdgeUses.end(); ++it)
    {
    (*it)->Destroy();
    (*it)->GetModelEdge()->DestroyModelEdgeUse(*it);
    }
  return true;
}

int vtkModelLoopUse::GetType()
{
  return vtkModelLoopUseType;
}

int vtkModelLoopUse::GetNumberOfModelEdgeUses()
{
  return this->GetNumberOfAssociations(vtkModelEdgeUseType);
}

vtkModelFace* vtkModelLoopUse::GetModelFace()
{
  vtkModelItemIterator* iter = this->NewIterator(vtkModelFaceUseType);
  iter->Begin();
  vtkModelFace* Face = 0;
  if(!iter->IsAtEnd())
    {
    if(vtkModelFaceUse* FaceUse = vtkModelFaceUse::SafeDownCast(iter->GetCurrentItem()))
      {
      Face = FaceUse->GetModelFace();
      }
    }
  iter->Delete();
  return Face;
}

vtkModelEdgeUse* vtkModelLoopUse::GetModelEdgeUse(int index)
{
  vtkSmartPointer<vtkModelItemIterator> edgeUses;
  edgeUses.TakeReference(this->NewIterator(vtkModelEdgeUseType));
  int counter = 0;
  for(edgeUses->Begin();!edgeUses->IsAtEnd();edgeUses->Next())
    {
    if(counter == index)
      {
      return vtkModelEdgeUse::SafeDownCast(edgeUses->GetCurrentItem());
      }
    counter++;
    }
  vtkWarningMacro("Bad index.");
  return NULL;
}

vtkModelItemIterator* vtkModelLoopUse::NewModelEdgeUseIterator()
{
  return this->NewIterator(vtkModelEdgeUseType);
}

void vtkModelLoopUse::InsertModelEdgeUse(int Index, vtkModelEdgeUse* EdgeUse)
{
  this->AddAssociationInPosition(Index, EdgeUse);
}

void vtkModelLoopUse::RemoveModelEdgeUseAssociation(vtkModelEdgeUse* EdgeUse)
{
  this->RemoveAssociation(EdgeUse);
}

void vtkModelLoopUse::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

int vtkModelLoopUse::GetModelEdgeUseIndex(vtkModelEdgeUse* edgeUse)
{
  vtkSmartPointer<vtkModelItemIterator> edgeUses;
  edgeUses.TakeReference(this->NewModelEdgeUseIterator());
  int index = 0;
  for(edgeUses->Begin();!edgeUses->IsAtEnd();edgeUses->Next())
    {
    if(edgeUses->GetCurrentItem() == edgeUse)
      {
      return index;
      }
    index++;
    }
  return -1;
}

int vtkModelLoopUse::GetNumberOfModelVertices()
{
  std::set<vtkModelVertex*> vertices;
  vtkModelItemIterator* edgeUses = this->NewModelEdgeUseIterator();
  for(edgeUses->Begin();!edgeUses->IsAtEnd();edgeUses->Next())
    {
    vtkModelEdgeUse* edgeUse =
      vtkModelEdgeUse::SafeDownCast(edgeUses->GetCurrentItem());
    vtkModelItemIterator* vertexUses =
      edgeUse->NewIterator(vtkModelVertexUseType);
    for(vertexUses->Begin();!vertexUses->IsAtEnd();vertexUses->Next())
      {
      vtkModelVertexUse* vertexUse =
        vtkModelVertexUse::SafeDownCast(vertexUses->GetCurrentItem());
      vertices.insert(vertexUse->GetModelVertex());
      }
    vertexUses->Delete();
    }
  edgeUses->Delete();
  return static_cast<int>(vertices.size());
}

void vtkModelLoopUse::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

