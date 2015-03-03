//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkModelEdgeUse.h"
#include "vtkModelFace.h"
#include "vtkModelFaceUse.h"
#include "vtkModelItemIterator.h"
#include "vtkModelVertex.h"
#include "vtkModelVertexUse.h"


#include "vtkModelLoopUse.h"

bool IsSingleFaceModelValid(vtkDiscreteModel* model, int numberOfEdges, int* edgeDirections);

int main()
{
  vtkDiscreteModel* model = vtkDiscreteModel::New();
  vtkModelEdge* SingleEdgeModelEdge = model->BuildModelEdge(0, 0);
  vtkModelEdge* Edges[3] = {SingleEdgeModelEdge, NULL, NULL};
  int edgeDirections[3] = {1, 0, 0};
  /*vtkModelFace* SingleEdgeModelFace = */model->BuildModelFace(1, Edges, edgeDirections);
  bool Status = !IsSingleFaceModelValid(model, 1, edgeDirections);
  if(Status)
    {
    vtkGenericWarningMacro("The single face model with a single model edge is NOT valid!");
    }
  else
    {
    vtkGenericWarningMacro("SUCCESS: The single face model with a single model edge seems to be valid.");
    }

  model->Reset();

  // since we don't have a grid we put in junk for the point id for the model vertex
  // it should not matter since we are not testing any of the grid/model classification
  vtkModelVertex* Vertex0 = model->BuildModelVertex(-3);
  vtkModelVertex* Vertex1 = model->BuildModelVertex(-4);
  Edges[0] = model->BuildModelEdge(Vertex0, Vertex1);
  Edges[1] = model->BuildModelEdge(Vertex0, Vertex1);
  /*vtkModelFace* Face0 = */model->BuildModelFace(2, Edges, edgeDirections);

  Status = !IsSingleFaceModelValid(model, 2, edgeDirections);
  if(Status)
    {
    vtkGenericWarningMacro("The single face model is NOT valid!");
    }
  else
    {
    vtkGenericWarningMacro("SUCCESS: The single face model seems to be valid.");
    }

  model->Reset();
  model->Delete();

  if(Status)
    {
    vtkGenericWarningMacro("Test failed.");
    }
  return Status;
}

// check to see that the model is valid for a simple, single model face.
// numberOfEdges is the number of edges that should be in the model.
bool IsSingleFaceModelValid(vtkDiscreteModel* model, int numberOfEdges, int* edgeDirections)
{
  // see if each model vertex has only a single model vertex use,
  // each model vertex use has 2 model edge uses, and each edge
  // use has a single edge
  if(numberOfEdges == 1 &&
     model->GetNumberOfAssociations(vtkModelVertexType) > 1 && // can only be 0 or 1 model vertices
     model->GetNumberOfAssociations(vtkModelEdgeType) != 1)
    {
    printf("Number of model edges is %d and number of model vertices is %d\n",
           model->GetNumberOfAssociations(vtkModelVertexType),
           model->GetNumberOfAssociations(vtkModelEdgeType));
    vtkGenericWarningMacro("Wrong number of model vertices for single model edge.");
    return false;
    }
  else if(numberOfEdges > 1 && ( model->GetNumberOfAssociations(vtkModelEdgeType) != numberOfEdges ||
                                 model->GetNumberOfAssociations(vtkModelVertexType) != numberOfEdges) )
    {
    vtkGenericWarningMacro("Wrong number of model edges and/or model vertices.");
    return false;
    }
  vtkModelItemIterator* Vertices = model->NewIterator(vtkModelVertexType);
  for(Vertices->Begin();!Vertices->IsAtEnd();Vertices->Next())
    {
    vtkModelVertex* Vertex =
      vtkModelVertex::SafeDownCast(Vertices->GetCurrentItem());
    // for a single model face it will have two model face uses
    // which will end up creating two model vertex uses for each vertex
    if(Vertex->GetNumberOfModelVertexUses() != 2)
      {
      Vertices->Delete();
      return false;
      }
    vtkModelItemIterator* VertexUses = Vertex->NewModelVertexUseIterator();
    for(VertexUses->Begin();!VertexUses->IsAtEnd();VertexUses->Next())
      {
      vtkModelVertexUse* VertexUse =
        vtkModelVertexUse::SafeDownCast(VertexUses->GetCurrentItem());
      if(VertexUse->GetNumberOfModelEdgeUses() != 2)
        {
        Vertices->Delete();
        VertexUses->Delete();
        return false;
        }
      }
    VertexUses->Delete();
    }
  Vertices->Delete();

  // checking model face information
  if(model->GetNumberOfAssociations(vtkModelFaceType) != 1)
    {
    vtkGenericWarningMacro("There should be a single model face but there were "
                           << model->GetNumberOfAssociations(vtkModelFaceType) << " model faces.");
    return false;
    }
  vtkModelItemIterator* Faces = model->NewIterator(vtkModelFaceType);
  Faces->Begin();
  vtkModelFace* Face = vtkModelFace::SafeDownCast(Faces->GetCurrentItem());
  Faces->Delete();
  vtkModelFaceUse* FaceUse0 = Face->GetModelFaceUse(0);
  vtkModelFaceUse* FaceUse1 = Face->GetModelFaceUse(1);

  // checking model edge information.
  if(model->GetNumberOfAssociations(vtkModelEdgeType) != numberOfEdges)
    {
    vtkGenericWarningMacro("There should be two model edges but there were "
                           << model->GetNumberOfAssociations(vtkModelEdgeType) << " model edges.");
    return false;
    }

  int counter = 0;
  vtkModelItemIterator* Edges = model->NewIterator(vtkModelEdgeType);
  for(Edges->Begin();!Edges->IsAtEnd();Edges->Next(),counter++)
    {
    vtkModelEdge* Edge = vtkModelEdge::SafeDownCast(Edges->GetCurrentItem());
    if(Edge->GetNumberOfModelEdgeUses() != 2)
      {
      Edges->Delete();
      vtkGenericWarningMacro("Model edge had " << Edge->GetNumberOfModelEdgeUses()
                             << " model edge uses but should only have 2 for a simple single model face.");
      return false;
      }
    vtkModelEdgeUse* EdgeUse0 = Edge->GetModelEdgeUse(0);
    vtkModelEdgeUse* EdgeUse1 = Edge->GetModelEdgeUse(1);
    if(edgeDirections[counter] == 0)
      {
      EdgeUse0 = EdgeUse1;
      EdgeUse1 = Edge->GetModelEdgeUse(0);
      }
    if(EdgeUse0->GetPairedModelEdgeUse() != EdgeUse1 ||
       EdgeUse1->GetPairedModelEdgeUse() != EdgeUse0)
      {
      Edges->Delete();
      vtkGenericWarningMacro("The edge use pairs do not match.");
      return false;
      }
    if(EdgeUse0->GetDirection() == EdgeUse1->GetDirection())
      {
      Edges->Delete();
      vtkGenericWarningMacro("The edge use pairs do not have opposite directions.");
      return false;
      }
    // ASSUME THERE IS ONLY 1 Loop for the face
    if(EdgeUse0->GetModelLoopUse() != FaceUse0->GetOuterLoopUse())
      {
      Edges->Delete();
      vtkGenericWarningMacro("Something wrong with the edge use to loop or face to loop adjacency for the negative side.");
      return false;
      }
    if(EdgeUse1->GetModelLoopUse() != FaceUse1->GetOuterLoopUse())
      {
      Edges->Delete();
      vtkGenericWarningMacro("Something wrong with the edge use to loop or face to loop adjacency for the positive side.");
      return false;
      }
    }
  Edges->Delete();

  return true;
}
