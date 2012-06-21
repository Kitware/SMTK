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
#include "vtkModelFace.h"

#include "vtkModelItemIterator.h"
#include "vtkModelItemGenericIterator.h"
#include "vtkModel.h"
#include "vtkModelEdge.h"
#include "vtkModelEdgeUse.h"
#include "vtkModelFaceUse.h"
#include "vtkModelLoopUse.h"
#include "vtkModelShellUse.h"
#include "vtkModelVertexUse.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

#include <set>

vtkCxxRevisionMacro(vtkModelFace, "");

//-----------------------------------------------------------------------------
vtkModelFace::vtkModelFace()
{
  // We build the face uses automatically because we know all we need
  // to know about them now to build them.
  vtkModelFaceUse* FaceUse0 = vtkModelFaceUse::New();
  this->AddAssociation(FaceUse0->GetType(), FaceUse0, vtkModelFaceType);
  FaceUse0->Delete();
  vtkModelFaceUse* FaceUse1 = vtkModelFaceUse::New();
  this->AddAssociation(FaceUse1->GetType(), FaceUse1, vtkModelFaceType);
  FaceUse1->Delete();
}

//-----------------------------------------------------------------------------
vtkModelFace::~vtkModelFace()
{
}

//-----------------------------------------------------------------------------
bool vtkModelFace::IsDestroyable()
{
  for(int i=0;i<2;i++)
    {
    vtkModelFaceUse* FaceUse = this->GetModelFaceUse(i);
    if(FaceUse->GetModelShellUse())
      {
      return 0;
      }
    }
  return 1;
}

//-----------------------------------------------------------------------------
bool vtkModelFace::Destroy()
{
  this->GetModel()->InvokeModelGeometricEntityEvent(
    ModelGeometricEntityAboutToDestroy, this);
  vtkModelFaceUse* faceUse0 = this->GetModelFaceUse(0);
  vtkModelFaceUse* faceUse1 = this->GetModelFaceUse(1);
  if(!faceUse0->Destroy())
    {
    vtkErrorMacro("Problem destroying face's face use 0.");
    return 0;
    }
  if(!faceUse1->Destroy())
    {
    vtkErrorMacro("Problem destroying face's face use 1.");
    return 0;
    }
  this->RemoveAllAssociations(vtkModelFaceUseType);
  return 1;
}

//-----------------------------------------------------------------------------
void vtkModelFace::Initialize(
  int NumEdges, vtkModelEdge** Edges, int* EdgeDirections,
  vtkIdType ModelFaceId)
{
  bool blockSignal = this->GetModel()->GetBlockModelGeometricEntityEvent();
  this->GetModel()->SetBlockModelGeometricEntityEvent(true);
  this->Superclass::Initialize(ModelFaceId);
  if (NumEdges)
    {
    this->AddLoop(NumEdges, Edges, EdgeDirections);
    }
  this->GetModel()->SetBlockModelGeometricEntityEvent(blockSignal);
}

//-----------------------------------------------------------------------------
// "end" of 1 combines with "beginning" of 2
void vtkModelFace::CombineModelVertexUses(
  vtkModelEdgeUse* edgeUse1, vtkModelEdgeUse* edgeUse2)
{
  vtkModelVertexUse* eu1KeepVU = edgeUse1->GetModelVertexUse(0);
  vtkModelVertexUse* eu1CombineVU = edgeUse1->GetModelVertexUse(1);
  vtkModelVertexUse* eu2CombineVU = edgeUse2->GetModelVertexUse(0);

  if(eu1CombineVU->GetModelVertex() != eu2CombineVU->GetModelVertex())
    {
    vtkErrorMacro("Vertex uses are not associated with the same model vertex.");
    return;
    }
  if(eu1CombineVU->GetNumberOfModelEdgeUses() == 1)
    {
    edgeUse1->SetModelVertexUses(eu1KeepVU, eu2CombineVU);
    // temporarily register EU1CombineVU so that I can propery delete it
    eu1CombineVU->Register(this);
    eu1CombineVU->Destroy();
    eu1CombineVU->UnRegister(this);
    }
  else if(eu2CombineVU->GetNumberOfModelEdgeUses() == 1)
    {
    vtkWarningMacro("why am i in here\n");
    vtkModelVertexUse* eu2KeepVU = edgeUse2->GetModelVertexUse(1);
    edgeUse2->SetModelVertexUses(eu1CombineVU, eu2KeepVU);
    // temporarily register EU2CombineVU so that I can propery delete it
    eu2CombineVU->Register(this);
    eu2CombineVU->Destroy();
    eu2CombineVU->UnRegister(this);
    }
  else
    {
    vtkErrorMacro("Bad input topology.");
    }
}

//-----------------------------------------------------------------------------
int vtkModelFace::GetType()
{
  return vtkModelFaceType;
}

//-----------------------------------------------------------------------------
vtkModelFaceUse* vtkModelFace::GetModelFaceUse(int direction)
{
  vtkModelItemIterator* iter = this->NewIterator(vtkModelFaceUseType);
  iter->Begin();
  if (direction)
    {
    iter->Next();
    }
  vtkModelFaceUse* ret = vtkModelFaceUse::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return ret;
}

//-----------------------------------------------------------------------------
int vtkModelFace::GetNumberOfModelRegions()
{
  int result = 0;
  for(int direction=0;direction<2;direction++)
    {
    vtkModelFaceUse* FaceUse = this->GetModelFaceUse(direction);
    if(vtkModelShellUse* ShellUse = FaceUse->GetModelShellUse())
      {
      result++;
      }
    }
  return result;
}

//-----------------------------------------------------------------------------
vtkModelRegion* vtkModelFace::GetModelRegion(int direction)
{
  vtkModelFaceUse* FaceUse = this->GetModelFaceUse(direction);
  vtkModelShellUse* ShellUse = FaceUse->GetModelShellUse();
  if(!ShellUse)
    {
    return 0;
    }
  return ShellUse->GetModelRegion();
}

//-----------------------------------------------------------------------------
int vtkModelFace::GetNumberOfModelEdges()
{
  std::set<vtkIdType> edgeIds;
  vtkModelItemIterator* loopUses =
    this->GetModelFaceUse(0)->NewLoopUseIterator();
  for(loopUses->Begin();!loopUses->IsAtEnd();loopUses->Next())
    {
    vtkModelLoopUse* loopUse =
      vtkModelLoopUse::SafeDownCast(loopUses->GetCurrentItem());
    vtkModelItemIterator* edgeUses = loopUse->NewModelEdgeUseIterator();
    for(edgeUses->Begin();!edgeUses->IsAtEnd();edgeUses->Next())
      {
      vtkModelEdge* edge = vtkModelEdgeUse::SafeDownCast(
        edgeUses->GetCurrentItem())->GetModelEdge();
      edgeIds.insert(edge->GetUniquePersistentId());
      }
    edgeUses->Delete();
    }
  loopUses->Delete();
  return static_cast<int>(edgeIds.size());
}

//-----------------------------------------------------------------------------
vtkModelItemIterator* vtkModelFace::NewAdjacentModelEdgeIterator()
{
  vtkModelItemGenericIterator* edgeIterator =
    vtkModelItemGenericIterator::New();

  // Loop through  loops of the 0th face use
  vtkModelItemIterator *liter = this->GetModelFaceUse(0)->NewLoopUseIterator();
  for (liter->Begin();!liter->IsAtEnd(); liter->Next())
    {
    vtkModelItemIterator* edgeUses =
      vtkModelLoopUse::SafeDownCast(liter->GetCurrentItem())
      ->NewModelEdgeUseIterator();
    for(edgeUses->Begin();!edgeUses->IsAtEnd();edgeUses->Next())
      {
      vtkModelEdgeUse* edgeUse =
        vtkModelEdgeUse::SafeDownCast(edgeUses->GetCurrentItem());
      edgeIterator->AddModelItem(edgeUse->GetModelEdge());
      }
    edgeUses->Delete();
    }
  liter->Delete();
  return edgeIterator;
}

//-----------------------------------------------------------------------------
void vtkModelFace::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

//-----------------------------------------------------------------------------
void vtkModelFace::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//-----------------------------------------------------------------------------
void vtkModelFace::AddLoop(int numEdges, vtkModelEdge** edges,
                           int* edgeDirections)
{
  if(!numEdges)
    {
    return;
    }

  std::vector<vtkModelEdgeUse*> edgeUses(numEdges);
  for(int i=0;i<numEdges;i++)
    {
    if(edges[i]->GetNumberOfModelEdgeUses() == 2 &&
       edges[i]->GetModelEdgeUse(0)->GetNumberOfAssociations(vtkModelLoopUseType) == 0)
      {
      // this is the default edge use associated with the model edge
      edgeUses[i] = edges[i]->GetModelEdgeUse(edgeDirections[i]);
      }
    else
      {
      vtkModelEdgeUse* edgeUse = edges[i]->BuildModelEdgeUsePair();
      edgeUses[i] = edgeDirections[i] == 1 ? edgeUse : edgeUse->GetPairedModelEdgeUse();
      }
    }

  if(numEdges>1)
    {
    for(int i=0;i<numEdges;i++)
      {
      // we have to get rid of duplicate model vertex uses
      // so that the model topology is correct because we didn't know
      // how model edge uses connected up until we created a loop use.
      // note that the edge use pair directions are already opposite
      if(i == 0)
        {
        this->CombineModelVertexUses(edgeUses[numEdges-1],edgeUses[0]);
        this->CombineModelVertexUses(edgeUses[0]->GetPairedModelEdgeUse(),
                                     edgeUses[numEdges-1]->GetPairedModelEdgeUse());
        }
      else
        {
        this->CombineModelVertexUses(edgeUses[i-1], edgeUses[i]);
        this->CombineModelVertexUses(edgeUses[i]->GetPairedModelEdgeUse(),
                                     edgeUses[i-1]->GetPairedModelEdgeUse());
        }
      }
    }
  else
    { // single model edge
    // This logic seems to be redundant since the exact same logic is already applied
    // at the begining of this method.
    /*
    if(edges[0]->GetNumberOfModelEdgeUses() == 2 &&
       edges[0]->GetModelEdgeUse(0)->GetNumberOfAssociations(vtkModelLoopUseType) == 0)
      {  // this is the default edge use associated with the model edge
      edgeUses[0] = edges[0]->GetModelEdgeUse(edgeDirections[0]);
      }
    else
      {
      vtkModelEdgeUse* edgeUse = edges[0]->BuildModelEdgeUsePair();
      edgeUses[0] = edgeDirections[0] == 1 ? edgeUse : edgeUse->GetPairedModelEdgeUse();
      }
    */
    if(edgeUses[0]->GetModelVertexUse(0) != edgeUses[0]->GetModelVertexUse(1))
      {
      this->CombineModelVertexUses(edgeUses[0],edgeUses[0]);
      this->CombineModelVertexUses(edgeUses[0]->GetPairedModelEdgeUse(),
                                   edgeUses[0]->GetPairedModelEdgeUse());
      }
    }

  // the model edge uses should all be set assuming this model face is not adjacent
  // to any other model faces so now build model loop uses if there are any
  // adjacent model edges
  vtkModelLoopUse* loopUse1 = vtkModelLoopUse::New();
  this->GetModelFaceUse(0)->AddLoopUse(loopUse1);
  loopUse1->InsertModelEdgeUse(0, edgeUses[0]->GetPairedModelEdgeUse());
  loopUse1->Delete();

  vtkModelLoopUse* loopUse2 = vtkModelLoopUse::New();
  this->GetModelFaceUse(1)->AddLoopUse(loopUse2);
  loopUse2->InsertModelEdgeUse(0, edgeUses[0]);
  loopUse2->Delete();

  for(int i=1;i<numEdges;i++)
    {
    // loopUse1 is for for face use on "other" side, e.g. faceUse0
    loopUse1->InsertModelEdgeUse(i, edgeUses[numEdges-i]->GetPairedModelEdgeUse());
    loopUse2->InsertModelEdgeUse(i, edgeUses[i]);
    }
  this->GetModel()->InvokeModelGeometricEntityEvent(
    ModelGeometricEntityBoundaryModified, this);
}

//-----------------------------------------------------------------------------
int vtkModelFace::GetNumberOfHoles()
{
  vtkSmartPointer<vtkModelItemIterator> loopUses;
  loopUses.TakeReference(this->GetModelFaceUse(0)->NewLoopUseIterator());
  loopUses->Begin();
  // we skip the first one since that is the outer loop.
  // if there's only one loop then there aren't any topological holes!
  if(loopUses->IsAtEnd())
    {
    return 0;
    }
  int numberOfHoles = 0;
  for(loopUses->Next();!loopUses->IsAtEnd();loopUses->Next())
    {
    vtkModelLoopUse* loopUse =
      vtkModelLoopUse::SafeDownCast(loopUses->GetCurrentItem());
    std::set<vtkModelEdge*> edges;
    vtkModelItemIterator* edgeUses = loopUse->NewModelEdgeUseIterator();
    for(edgeUses->Begin();!edgeUses->IsAtEnd();edgeUses->Next())
      {
      vtkModelEdgeUse* edgeUse =
        vtkModelEdgeUse::SafeDownCast(edgeUses->GetCurrentItem());
      vtkModelEdge* edge = edgeUse->GetModelEdge();
      std::set<vtkModelEdge*>::iterator it = edges.find(edge);
      if(it == edges.end())
        {
        edges.insert(edge);
        }
      else
        {
        edges.erase(it);
        }
      }
    if(edges.empty() == false)
      {
      // this is a hole because there's at least one edge that
      // the loop use is not using 2 of the edge's edge uses
      numberOfHoles++;
      }
    edgeUses->Delete();
    }

  return numberOfHoles;
}

//-----------------------------------------------------------------------------
int vtkModelFace::GetNumberOfDegenerateLoops()
{
  int numberOfLoopUses = this->GetModelFaceUse(0)->GetNumberOfLoopUses();
  return numberOfLoopUses - this->GetNumberOfHoles() - 1;
}
