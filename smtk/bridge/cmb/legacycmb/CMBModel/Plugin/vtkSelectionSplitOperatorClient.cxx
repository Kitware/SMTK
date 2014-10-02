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

#include "vtkSelectionSplitOperatorClient.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelFace.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkModelEdge.h"
#include "vtkModelFaceUse.h"
#include "vtkModelShellUse.h"
#include "vtkModelVertex.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkSMIdTypeVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#
#include "vtkSMProxyManager.h"
#include "vtkSMSourceProxy.h"

#include <map>

vtkStandardNewMacro(vtkSelectionSplitOperatorClient);

vtkSelectionSplitOperatorClient::vtkSelectionSplitOperatorClient()
{
}

vtkSelectionSplitOperatorClient::~vtkSelectionSplitOperatorClient()
{
}

bool vtkSelectionSplitOperatorClient::Operate(
  vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy,
  vtkSMProxy* SelectionSourceProxy)
{
  if(this->AbleToOperate(Model) == 0 || SelectionSourceProxy == NULL ||
     ServerModelProxy == NULL)
    {
    return 0;
    }
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy =
    vtkSMOperatorProxy::SafeDownCast(
      manager->NewProxy("CMBModelGroup", "SelectionSplitOperator"));
  if(!OperatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
    }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  OperatorProxy->Operate(Model, ServerModelProxy, SelectionSourceProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("OperateSucceeded"));

  OperatorProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  if(!Succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    OperatorProxy->Delete();
    return 0;
    }

  // now update the information on the client
  vtkSMIdTypeVectorProperty* ModifiedPairs =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("ModifiedPairIDs"));
  vtkSMIdTypeVectorProperty* SelectedIds =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("CompletelySelectedIDs"));

  OperatorProxy->UpdateVTKObjects();
  OperatorProxy->UpdatePropertyInformation();

  // ModifiedPairs should now have the actual ids of the
  // modified geometric model entities.
  if(this->GetModifiedPairIDs()->GetNumberOfTuples())
    {
    this->GetModifiedPairIDs()->Reset();
    this->GetModifiedPairIDs()->SetNumberOfComponents(2);
    vtkWarningMacro("Possible problem with already having ModifiedPairIDs.");
    }

  vtkSMIdTypeVectorProperty* CurrentExistingFaceId =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("CurrentExistingFaceId"));
  vtkSMIdTypeVectorProperty* SplitEdgeVertIds =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("SplitEdgeVertIds"));
  vtkSMIdTypeVectorProperty* CreatedModelEdgeVertIDs =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("CreatedModelEdgeVertIDs"));
  vtkSMIdTypeVectorProperty* FaceEdgeLoopIDs =
    vtkSMIdTypeVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("FaceEdgeLoopIDs"));

  unsigned int NumberOfElements = ModifiedPairs->GetNumberOfElements();
  for(unsigned int ui=0;ui<NumberOfElements/2;ui++)
    {
    vtkIdType SourceId = ModifiedPairs->GetElement(ui*2);
    vtkIdType TargetId = ModifiedPairs->GetElement(ui*2+1);
    this->AddModifiedPair(SourceId, TargetId);
    // For now assume that we don't know the entity type.
    vtkModelEntity* SourceEntity = Model->GetModelEntity(SourceId);
    vtkDiscreteModelFace* SourceFace = vtkDiscreteModelFace::SafeDownCast(SourceEntity);
    if(SourceFace)
      {
      vtkModelFace* TargetFace = Model->BuildModelFace(0, 0, 0);
      for(int i=0;i<2;i++)
        {
        vtkModelFaceUse* SourceFaceUse = SourceFace->GetModelFaceUse(i);
        vtkModelShellUse* ShellUse = SourceFaceUse->GetModelShellUse();
        if(ShellUse)
          {
          ShellUse->AddModelFaceUse(TargetFace->GetModelFaceUse(i));
          }
        }
      if(TargetFace->GetUniquePersistentId() != TargetId)
        {
        vtkErrorMacro("Created model face ids do not match on server and client.");
        }
      // update Edges due to split of faces. We need to follow exactly (skipping Geometry)
      // what's happening on server side so that the Ids will match between server and client.
      bool hasEdges = SourceFace->GetNumberOfModelEdges() != 0;
      if(hasEdges)
        {
        CurrentExistingFaceId->SetElement(0, SourceId);
        OperatorProxy->UpdateVTKObjects();
        OperatorProxy->UpdatePropertyInformation();
        vtkSelectionSplitOperatorClient::UpdateSplitEdgeVertIds(
          Model, SplitEdgeVertIds, this->SplitEdgeMap, this->SplitVertMap);
        vtkSelectionSplitOperatorClient::UpdateCreatedModelEdgeVertIDs(
          Model, CreatedModelEdgeVertIDs, this->NewEdges, this->NewVerts);
        vtkSelectionSplitOperatorClient::UpdateFaceEdgeLoopIDs(
          Model, FaceEdgeLoopIDs);
        }
      }
    else
      {
      vtkWarningMacro("Currently only able to handle model faces.");
      OperatorProxy->Delete();
      return 0;
      }
    }

  // now just keep track of the completely selected IDs of objects
  if(this->GetCompletelySelectedIDs()->GetNumberOfTuples())
    {
    this->GetCompletelySelectedIDs()->Reset();
    this->GetCompletelySelectedIDs()->SetNumberOfComponents(1);
    vtkWarningMacro("Possible problem with already having CompletelySelectedIDs.");
    }
  NumberOfElements = SelectedIds->GetNumberOfElements();
  for(unsigned int ui=0;ui<NumberOfElements;ui++)
    {
    vtkIdType SelectedId = SelectedIds->GetElement(ui);
    this->GetCompletelySelectedIDs()->InsertNextValue(SelectedId);
    }

  OperatorProxy->Delete();
  return 1;
}

void vtkSelectionSplitOperatorClient::UpdateSplitEdgeVertIds(
    vtkDiscreteModel* Model, vtkSMIdTypeVectorProperty* SplitEdgeVertIds,
    std::map<vtkIdType, std::vector<vtkIdType> >& SplitEdgeMap,
    std::map<vtkIdType, std::vector<vtkIdType> >& SplitVertMap)
{
  vtkModelEdge* edgeEnt;
  // The map of <OldEdgeId, NewVertId, NewEdgId>
  unsigned int NumberOfElements = SplitEdgeVertIds->GetNumberOfElements();
  for(unsigned int ev=0;ev<NumberOfElements/3;ev++)
    {
    vtkIdType oldEdgeId = SplitEdgeVertIds->GetElement(ev*3);
    vtkIdType nVertId = SplitEdgeVertIds->GetElement(ev*3+1);
    vtkIdType nEdgeId = SplitEdgeVertIds->GetElement(ev*3+2);
    if(nVertId < 0 || Model->GetModelEntity(vtkModelVertexType, nVertId))
      {
      //vtkWarningMacro("Not valid new vertex Id from server after split on server.");
      continue;
      }
    edgeEnt = vtkModelEdge::SafeDownCast(
      Model->GetModelEntity(vtkModelEdgeType, oldEdgeId));
    if(!edgeEnt)
      {
      //vtkWarningMacro("Can't find model edge with this edge Id.");
      continue;
      }
     vtkModelVertex* vertex = Model->BuildModelVertex(-1, nVertId);
     SplitVertMap[oldEdgeId].push_back(nVertId);
     // If there is no vertex, means only a new vertex is created
    if(edgeEnt->GetAdjacentModelVertex(0) == 0)
      {
      edgeEnt->SplitModelEdgeLoop(vertex);
      }
    else
      {
      vtkModelEdge* newEdge = Model->BuildModelEdge(0,0);
      if(newEdge->GetUniquePersistentId() != nEdgeId)
        {
        //vtkWarningMacro("The new edge Id does not match between server and client.");
        continue;
        }
      edgeEnt->SplitModelEdge(vertex, newEdge);
      SplitEdgeMap[oldEdgeId].push_back(nEdgeId);
      }
    }
}

void vtkSelectionSplitOperatorClient::UpdateCreatedModelEdgeVertIDs(
    vtkDiscreteModel* Model, vtkSMIdTypeVectorProperty* CreatedModelEdgeVertIDs,
    std::vector<vtkIdType>& newEdges, std::vector<vtkIdType>& newVerts)
{
  // The map of <NewEdgeId, VertId1, VertId2>
  unsigned int NumberOfElements = CreatedModelEdgeVertIDs->GetNumberOfElements();
  for(unsigned int mev=0;mev<NumberOfElements/3;mev++)
    {
    vtkIdType newEdgeId = CreatedModelEdgeVertIDs->GetElement(mev*3);
    vtkIdType VertId1 = CreatedModelEdgeVertIDs->GetElement(mev*3+1);
    vtkIdType VertId2 = CreatedModelEdgeVertIDs->GetElement(mev*3+2);
    vtkModelVertex * v0 = dynamic_cast<vtkModelVertex *>(
      Model->GetModelEntity(vtkModelVertexType, VertId1));
    if(v0==NULL)
      {
      v0 = Model->BuildModelVertex(-1, VertId1);
      newVerts.push_back(VertId1);
      }
    vtkModelVertex * v1 = dynamic_cast<vtkModelVertex *>(
      Model->GetModelEntity(vtkModelVertexType, VertId2));
    if(v1==NULL)
      {
      v1 = Model->BuildModelVertex(-1, VertId2);
      newVerts.push_back(VertId2);
      }
    if(v0 && v1)
      {
      vtkModelEdge* newEdge = Model->BuildModelEdge(v0, v1);
      if(newEdge->GetUniquePersistentId() != newEdgeId)
        {
        //vtkWarningMacro("The new edge Id does not match between server and client.");
        continue;
        }
      newEdges.push_back(newEdgeId);
      }
    else
      {
      //vtkWarningMacro("Not valid vertex Ids for creating new edges on client.");
      continue;
      }
    }
}

void vtkSelectionSplitOperatorClient::UpdateFaceEdgeLoopIDs(
    vtkDiscreteModel* Model, vtkSMIdTypeVectorProperty* FaceEdgeLoopIDs)
{
  // The map of <faceId, nLoops, [nEdges, (gedges[n]...), (orientations[n]...)]>
  unsigned int NumberOfElements = FaceEdgeLoopIDs->GetNumberOfElements();
  vtkModelEdge* gedge;
  vtkIdType gid;
  for(unsigned int n=0;n<NumberOfElements;)
    {
    vtkIdType faceId = FaceEdgeLoopIDs->GetElement(n++);
    vtkIdType nLoops = FaceEdgeLoopIDs->GetElement(n++);
    vtkModelFace* currentFace = dynamic_cast<vtkModelFace *>(
      Model->GetModelEntity(vtkModelFaceType, faceId));
    currentFace->DestroyLoopUses();
    for(vtkIdType l=0; l<nLoops && n<NumberOfElements; l++)
      {
      std::vector<int> orientations;
      std::vector<vtkModelEdge *> gedges;
      vtkIdType nEdges = FaceEdgeLoopIDs->GetElement(n++);
      orientations.resize(nEdges);
      gedges.resize(nEdges);
      for(vtkIdType e=0; e<nEdges && n<NumberOfElements; e++)
        {
        gid = FaceEdgeLoopIDs->GetElement(n++);
        gedge = dynamic_cast<vtkModelEdge *>
          (Model->GetModelEntity(vtkModelEdgeType, gid));
        gedges[e] = gedge;
        orientations[e] = FaceEdgeLoopIDs->GetElement(n++);
        }
      currentFace->AddLoop(nEdges, &gedges.front(), &orientations.front());
      }
    }
}

void vtkSelectionSplitOperatorClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
