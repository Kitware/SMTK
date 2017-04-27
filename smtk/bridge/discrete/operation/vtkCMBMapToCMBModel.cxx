//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBMapToCMBModel.h"

#include "math.h"
#include "vtkAbstractArray.h"
#include "vtkAlgorithm.h"
#include "vtkCell.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellLocator.h"
#include "vtkCharArray.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkInstantiator.h"
#include "vtkIntArray.h"
#include "vtkLine.h"
#include "vtkMath.h"
#include "vtkModelItemIterator.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"
#include <vtkSmartPointer.h>

#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <vector>

vtkStandardNewMacro(vtkCMBMapToCMBModel);

namespace
{

//==================================NEW METHOD OF CONSTRUCTING A CMBMODEL============================//

struct WalkableEdge
{
  WalkableEdge(
    const vtkIdType& _ptId1, const vtkIdType& _ptId2, const vtkIdType& _edgeId, const int& _dir)
    : ptId1(_ptId1)
    , ptId2(_ptId2)
    , edgeId(_edgeId)
    , dir(_dir)
  {
  }
  WalkableEdge(const vtkIdType& _ptId1, const vtkIdType& _ptId2, const vtkIdType& _edgeId)
    : ptId1(_ptId1)
    , ptId2(_ptId2)
    , edgeId(_edgeId)
    , dir(-1)
  {
  }
  WalkableEdge(const vtkIdType& _ptId1, const vtkIdType& _ptId2)
    : ptId1(_ptId1)
    , ptId2(_ptId2)
    , edgeId(-1)
    , dir(-1)
  {
  }

  ~WalkableEdge() {}

  //This comparison operator allows an edge to be viewed as an
  //ordered pair where -1 can be a wildcard in the second slot
  //the edgeid can also be -1 which is a wildcard
  // Examples: For any X and Y
  // (X, Y) will match (X,Y) and (X,-1)
  // (X,-1) will match (X,Y)
  // (X, Y) will NOT match (Y,X)
  // (-1,X) will NOT match (Y,X)
  // Note: -1 will not be viewed as a wildcard in ptId1
  bool operator<(const WalkableEdge& e) const
  {
    //Case where edgeid and ptid2 are wild
    if ((e.ptId2 == -1 || this->ptId2 == -1) && (e.edgeId == -1 && this->edgeId == -1))
    {
      return (this->ptId1 < e.ptId1);
    }
    //Case where only ptId2 is wild
    if (e.ptId2 == -1 || this->ptId2 == -1)
    {
      return (this->ptId1 < e.ptId1);
    }
    //Case where only edgeId is wild
    if (e.edgeId == -1 || this->edgeId == -1)
    {
      return ((this->ptId1 < e.ptId1) || (this->ptId1 == e.ptId1 && this->ptId2 < e.ptId2));
    }
    //Checks both orderings of endpoint1 and endpoint2. If they are both equal it makes sure
    //that the ids are not the same
    return ((this->ptId1 < e.ptId1) || (this->ptId1 == e.ptId1 && this->ptId2 < e.ptId2) ||
      (this->ptId1 == e.ptId1 && this->ptId2 == e.ptId2 && this->edgeId < e.edgeId));
  }

  const vtkIdType ptId1;
  const vtkIdType ptId2;
  const vtkIdType edgeId;
  //dir is only used when returning a WalkableEdge
  // -1 : invalid direction
  //  0 : counterclockwise
  //  1 : clockwise
  const int dir;
};

class WalkableLoop
{
public:
  //WalkableLoop(const vtkIdType& _loopId) : loopId(_loopId), numEdges(0), edge_arr(NULL), edge_dir_arr(NULL){}
  WalkableLoop()
    : numEdges(-1)
    , edge_arr(NULL)
    , edge_dir_arr(NULL)
    , edge_dir_flipped(false)
  {
  }
  ~WalkableLoop()
  {
    //If numEdges is initialized it means we created edge arrays
    if (this->numEdges != -1)
    {
      delete[] edge_arr;
      delete[] edge_dir_arr;
    }
  }

  void addEdge(
    const vtkIdType& ptId1, const vtkIdType& ptId2, const vtkIdType& arcId, vtkModelEdge* modelEdge)
  {
    order1.insert(WalkableEdge(ptId1, ptId2, arcId));
    order2.insert(WalkableEdge(ptId2, ptId1, arcId));
    arcId2ModelEdge[arcId] = modelEdge;
  }

  bool removeEdge(const vtkIdType& ptId1, const vtkIdType& ptId2, const vtkIdType& edgeId = -1)
  {
    //Try the first ordering of the edge
    std::set<WalkableEdge>::iterator foundOrder1 = order1.find(WalkableEdge(ptId1, ptId2, edgeId));
    std::set<WalkableEdge>::iterator foundOrder2 = order2.find(WalkableEdge(ptId2, ptId1, edgeId));
    if (foundOrder1 != order1.end() && foundOrder2 != order2.end())
    {
      order1.erase(foundOrder1);
      order2.erase(foundOrder2);
      return true;
    }
    //Try the second ordering of the edge
    foundOrder1 = order1.find(WalkableEdge(ptId2, ptId1, edgeId));
    foundOrder2 = order2.find(WalkableEdge(ptId1, ptId2, edgeId));
    if (foundOrder1 != order1.end() && foundOrder2 != order2.end())
    {
      order1.erase(foundOrder1);
      order2.erase(foundOrder2);
      return true;
    }
    return false; //The edge did not exist
  }

  // Finds an edge ids that share the same endpoints. ptId2 is a wildcard if it is equal to -1
  // Returns the a list of pairs of arc ids and their associated direction
  void findEdge(
    const vtkIdType& ptId1, const vtkIdType& ptId2, std::list<WalkableEdge>& foundArcsAndDirs) const
  {
    std::set<WalkableEdge>::const_iterator order1Iter =
      order1.lower_bound(WalkableEdge(ptId1, ptId2));
    std::set<WalkableEdge>::const_iterator order2Iter =
      order2.lower_bound(WalkableEdge(ptId1, ptId2));
    for (; order1Iter != order1.end(); order1Iter++)
    {
      // Make sure we are still within bounds of the arcs we want to find
      if (ptId1 != order1Iter->ptId1 || (ptId2 != -1 && ptId2 != order1Iter->ptId2))
        break;
      foundArcsAndDirs.push_back(WalkableEdge(order1Iter->ptId1, order1Iter->ptId2,
        order1Iter->edgeId, 1)); //return the arcId and clockwise direction
    }
    for (; order2Iter != order2.end(); order2Iter++)
    {
      // Make sure we are still within bounds of the arcs we want to find
      if (ptId1 != order2Iter->ptId1 || (ptId2 != -1 && ptId2 != order2Iter->ptId2))
        break;
      foundArcsAndDirs.push_back(WalkableEdge(order2Iter->ptId1, order2Iter->ptId2,
        order2Iter->edgeId, 0)); //return the arcId and counterclockwise direction
    }
  }

  // Calling this will destroy the loop's internal structure
  // Change the implementation if this class is needed for something
  // other than walking the loop once. This function does lazy evalutaion
  // if you call it more than once you always get the same answer
  bool getLoopOrdering(
    int& _numEdges, vtkModelEdge**& _edge_arr, int*& _edge_dir_arr, bool flip_edge_dir = false)
  {
    //Check to make sure this hasn't already run
    if (numEdges != -1)
    {
      if (numEdges == -2)
      {
        //The previous run of this function produced an error
        return false;
      }
      //If numEdges has been set then just return the previously computed values
      _numEdges = this->numEdges;
      _edge_arr = this->edge_arr;
      _edge_dir_arr = this->edge_dir_arr;
      if (flip_edge_dir != edge_dir_flipped)
      {
        //if we requested to flip the directions but we haven't done it
        edge_dir_flipped = flip_edge_dir;
        for (int i = 0; i < numEdges; i++)
        {
          edge_dir_arr[i] = 1 - edge_dir_arr[i];
        }
      }
      return true;
    }
    numEdges = static_cast<int>(order1.size());
    edge_arr = new vtkModelEdge*[numEdges];
    edge_dir_arr = new int[numEdges];
    int curr_insert_pos = 0; //keep track of where in the array we are inserting

    // Grab a random starting point from the clockwise order
    // Use this edge to start walking the loop
    std::set<WalkableEdge>::iterator curr = order1.begin();
    vtkIdType targetEndPoint =
      (*curr).ptId1;                  //The last segment to complete this loop should have this id
    vtkIdType currPt = (*curr).ptId2; //This will be the segment we start walking from

    edge_arr[curr_insert_pos] = arcId2ModelEdge[(*curr).edgeId]; //put in the first edge
    edge_dir_arr[curr_insert_pos] =
      1; //because we grab the first segment from order 1 the dir is always clockwise
    curr_insert_pos++;

    this->removeEdge((*curr).ptId1, (*curr).ptId2,
      (*curr).edgeId); //Remove the edge so we don't have to check it again

    // Start walking the rest of the edges
    while (order1.size() > 0)
    {
      //Find edges connected to the last endpoint
      std::list<WalkableEdge> foundEdges;
      this->findEdge(currPt, -1, foundEdges);

      if (foundEdges.size() == 1)
      {
        // Exactly one edge left starting from this point
        // This is what we expect
        WalkableEdge nextEdge = (*foundEdges.begin());
        edge_arr[curr_insert_pos] = arcId2ModelEdge[nextEdge.edgeId];
        edge_dir_arr[curr_insert_pos] = nextEdge.dir;
        curr_insert_pos++;
        //Remove the edge we just walked and initialize the next starting point
        this->removeEdge(nextEdge.ptId1, nextEdge.ptId2, nextEdge.edgeId);
        currPt = (currPt == nextEdge.ptId1) ? nextEdge.ptId2 : nextEdge.ptId1;

        if (currPt == targetEndPoint && order1.size() > 0)
        {
          //Error there are edges in the loop that are not part of the loop
          numEdges = -2;
          return false;
        }
      }
      //There is a non-manifold situation attempt to resolve
      else if (foundEdges.size() > 1)
      {
        //vtkWarningMacro("Walking non-manifold loop... attempting to resolve");
        // More than 2 edges use this endpoint
        // non-manifold situation
        std::list<WalkableEdge>::iterator nonmanifoldIter = foundEdges.begin();
        for (; nonmanifoldIter != foundEdges.end(); nonmanifoldIter++)
        {
          //We have found a loop within this loop it must be added next, also make sure the loop is going the same direction
          //as the previous edge, so this non-manifold sub loop doesn't cause the actual loop to interesect
          if ((*nonmanifoldIter).ptId1 == (*nonmanifoldIter).ptId2 &&
            edge_dir_arr[curr_insert_pos - 1] == (*nonmanifoldIter).dir)
          {
            //Break the list at the spot where the next edge is
            break;
          }
        }
        if (nonmanifoldIter == foundEdges.end())
        {
          //The situation is worse than we thought! Try the first edge in the list and
          //see if we get lucky
          nonmanifoldIter = foundEdges.begin();
        }
        WalkableEdge nextEdge = (*nonmanifoldIter);
        edge_arr[curr_insert_pos] = arcId2ModelEdge[nextEdge.edgeId];
        edge_dir_arr[curr_insert_pos] = nextEdge.dir;
        curr_insert_pos++;
        //Remove the edge we just walked and initialize the next starting point
        this->removeEdge(nextEdge.ptId1, nextEdge.ptId2, nextEdge.edgeId);
        currPt = (currPt == nextEdge.ptId1) ? nextEdge.ptId2 : nextEdge.ptId1;
      }
      else
      {
        numEdges = -2;
        return false;
        // The loop is not complete and we can't find a continuation
        // incomplete loop
      }
    }
    if (currPt == targetEndPoint)
    {
      //the loop we just walked was valid
      //everything was connected and there were no
      //non-manifold edges. Set the return value to the created
      //lists
      _numEdges = this->numEdges;
      _edge_arr = this->edge_arr;
      _edge_dir_arr = this->edge_dir_arr;
      if (flip_edge_dir != edge_dir_flipped)
      {
        //if we requested to flip the directions but we haven't done it
        edge_dir_flipped = flip_edge_dir;
        for (int i = 0; i < numEdges; i++)
        {
          edge_dir_arr[i] = 1 - edge_dir_arr[i];
        }
      }
      return true;
    }
    numEdges = -2;
    return false;
  }

private:
  //These are the variables that will ultimately be the output of
  //this class. Store them for easy access
  int numEdges;
  vtkModelEdge** edge_arr;
  int* edge_dir_arr;
  bool edge_dir_flipped;

  // We are going to view edges as unordered pairs
  // To make them searchable for each pair keep
  // 2 ordered pairs with different orders
  std::set<WalkableEdge> order1; //clockwise order
  std::set<WalkableEdge> order2; //counterclockwise order

  //Stores the arcs associated with this loop
  std::map<vtkIdType, vtkModelEdge*> arcId2ModelEdge;
};
}

vtkCMBMapToCMBModel::vtkCMBMapToCMBModel()
{
  this->OperateSucceeded = 0;
}

vtkCMBMapToCMBModel::~vtkCMBMapToCMBModel()
{
}

void vtkCMBMapToCMBModel::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}

void vtkCMBMapToCMBModel::Operate(vtkDiscreteModelWrapper* ModelWrapper, vtkAlgorithm* inputPolyAlg)
{
  if (!inputPolyAlg)
  {
    vtkErrorMacro("Passed in a null poly algorithm input.");
    return;
  }
  if (!ModelWrapper)
  {
    vtkErrorMacro("Passed in a null model.");
    return;
  }

  inputPolyAlg->Update();
  vtkPolyData* input = vtkPolyData::SafeDownCast(inputPolyAlg->GetOutputDataObject(0));
  if (!input)
  {
    vtkErrorMacro("Output from the input poly algorithm is not a polydata.");
    return;
  }

  if (input->GetNumberOfCells() == 0 || input->GetNumberOfPoints() == 0)
  {
    vtkErrorMacro("Unable to set the model to be an empty data set. This is most likely caused by "
                  "not having the relevant meshing support enabled");
    return;
  }

  vtkDiscreteModel* model = ModelWrapper->GetModel();
  model->Reset();

  DiscreteMesh mesh(input);
  model->SetMesh(mesh);

  vtkIdTypeArray* elementIds =
    vtkIdTypeArray::SafeDownCast(input->GetCellData()->GetArray("ElementIds"));
  if (!elementIds)
  {
    vtkErrorMacro("Input is not in the appropriate map format. There are no elementIds");
    return;
  }

  //=================Create Cell Mappings ====================//

  std::map<vtkIdType, vtkSmartPointer<vtkIdList> > arcId2CellIds;  // Needed to create edge geometry
  std::map<vtkIdType, vtkSmartPointer<vtkIdList> > polyId2CellIds; // Needed to create face geometry
  std::map<vtkIdType, vtkModelVertex*>
    nodeId2ModelVertex; // Needed to quickly build edges from just node ids

  // the CMB Model needs to know which cells belong to which node/edge/polygon
  // build up lists of these and map them to thier respective id
  vtkIdType faceId = 0, edgeId = 0; //faces are positive ids, edges are negative ids
  for (int cellId = 0; cellId < input->GetNumberOfCells(); cellId++)
  {
    vtkCell* cell = input->GetCell(cellId);
    vtkIdType elementId = elementIds->GetTuple1(cellId); //element id is the node/arc/poly id
    if (cell->GetCellType() == VTK_VERTEX)
    {
      // We only need to know one id to build a vertex, so just do that now
      // save it because we will later use this to build a model edge
      // Can not use elementId as uniquepersistantid, because it is not unique across nodes and arcs
      vtkDiscreteModelVertex* cmbVertex =
        vtkDiscreteModelVertex::SafeDownCast(model->BuildModelVertex(cell->GetPointId(0)));
      if (cmbVertex)
      {
        cmbVertex->CreateGeometry();
      }
      nodeId2ModelVertex[elementId] = cmbVertex;
    }
    if (cell->GetCellType() == VTK_LINE)
    {
      //Build a list of ModelEdge Cells and map them to arc ids
      std::map<vtkIdType, vtkSmartPointer<vtkIdList> >::iterator foundArc =
        arcId2CellIds.find(elementId);
      if (foundArc == arcId2CellIds.end())
      {
        foundArc =
          arcId2CellIds.insert(std::make_pair(elementId, vtkSmartPointer<vtkIdList>::New())).first;
      }
      foundArc->second->InsertNextId(edgeId);
      ++edgeId;
    }
    if (cell->GetCellType() == VTK_TRIANGLE)
    {
      //Build a list of ModelFace Cells and map them to polygon ids
      std::map<vtkIdType, vtkSmartPointer<vtkIdList> >::iterator foundPoly =
        polyId2CellIds.find(elementId);
      if (foundPoly == polyId2CellIds.end())
      {
        foundPoly =
          polyId2CellIds.insert(std::make_pair(elementId, vtkSmartPointer<vtkIdList>::New())).first;
      }
      foundPoly->second->InsertNextId(faceId);
      ++faceId;
    }
  }

  //=================Create Model Edges====================//

  vtkFieldData* fieldData = input->GetFieldData();
  vtkIdTypeArray* fieldArcIds = vtkIdTypeArray::SafeDownCast(fieldData->GetArray("ArcId"));
  vtkIdTypeArray* fieldLoop1 = vtkIdTypeArray::SafeDownCast(fieldData->GetArray("Loop1"));
  vtkIdTypeArray* fieldLoop2 = vtkIdTypeArray::SafeDownCast(fieldData->GetArray("Loop2"));
  vtkIdTypeArray* fieldEndpoint1 =
    vtkIdTypeArray::SafeDownCast(fieldData->GetArray("ArcEndpoint1"));
  vtkIdTypeArray* fieldEndpoint2 =
    vtkIdTypeArray::SafeDownCast(fieldData->GetArray("ArcEndpoint2"));
  vtkIdTypeArray* fieldLoopInfo = vtkIdTypeArray::SafeDownCast(fieldData->GetArray("LoopInfo"));

  //Create a walkable loop for every loop in the polydata
  std::vector<WalkableLoop> loopId2WalkableLoop;
  loopId2WalkableLoop.resize(fieldLoopInfo->GetNumberOfTuples(), WalkableLoop());

  for (int arcIndex = 0; arcIndex < fieldArcIds->GetNumberOfTuples(); arcIndex++)
  {
    vtkIdType arcId = fieldArcIds->GetTuple1(arcIndex);
    vtkIdType endpoint1 = fieldEndpoint1->GetTuple1(arcIndex);
    vtkIdType endpoint2 = fieldEndpoint2->GetTuple1(arcIndex);
    vtkIdType loop1 = fieldLoop1->GetTuple1(arcIndex);
    vtkIdType loop2 = fieldLoop2->GetTuple1(arcIndex);

    // Can not use arcId as uniquepersistantid, because it is not unique across nodes and arcs
    vtkModelEdge* edge =
      model->BuildModelEdge(nodeId2ModelVertex[endpoint1], nodeId2ModelVertex[endpoint2]);
    vtkDiscreteModelEdge::SafeDownCast(edge)->AddCellsToGeometry(arcId2CellIds[arcId]);

    //Add the edge to the loop that the arc is a part of
    if (loop1 != -1)
    {
      loopId2WalkableLoop[loop1].addEdge(endpoint1, endpoint2, arcId, edge);
    }
    if (loop2 != -1)
    {
      loopId2WalkableLoop[loop2].addEdge(endpoint1, endpoint2, arcId, edge);
    }
  }

  //=================Create Model Faces====================//

  std::map<vtkIdType, std::pair<std::list<vtkIdType>, vtkModelFace*> >
    polyId2InnerLoopsAndFaces; // on the first pass we can only create the outer loops
                               // remember the inner loops so we don't have to refind them later
                               // The inner loop ids are stored in a list of vtkIdTypes and the
  // list is associated via pair with the vtkModelFace they will belong to
  for (int loopIndex = 0; loopIndex < fieldLoopInfo->GetNumberOfTuples(); loopIndex++)
  {
    //Grab the inner and outer polygon that use this loop
    double* polyIds = vtkIdTypeArray::SafeDownCast(fieldLoopInfo)->GetTuple2(loopIndex);
    vtkIdType outerPoly = polyIds[0];
    vtkIdType innerPoly = polyIds[1];
    if (innerPoly > 0) // -1 and 0 are invalid ids
    {
      //save the inner poly for later
      std::map<vtkIdType, std::pair<std::list<vtkIdType>, vtkModelFace*> >::iterator foundPoly =
        polyId2InnerLoopsAndFaces.find(innerPoly);
      if (foundPoly == polyId2InnerLoopsAndFaces.end())
      {
        foundPoly =
          polyId2InnerLoopsAndFaces
            .insert(std::make_pair(innerPoly,
              std::pair<std::list<vtkIdType>, vtkModelFace*>(std::list<vtkIdType>(), NULL)))
            .first;
      }
      foundPoly->second.first.push_back(loopIndex);
    }
    if (outerPoly > 0) // -1 and 0 are invalid ids
    {
      //construct the outer loop for this face
      int numEdges;
      vtkModelEdge** edge_arr;
      int* edge_dir_arr;
      if (!loopId2WalkableLoop[loopIndex].getLoopOrdering(numEdges, edge_arr, edge_dir_arr))
      {
        vtkErrorMacro("Error with walking the outer loop");
        return;
      }
      vtkModelFace* face =
        model->BuildModelFace(numEdges, edge_arr, edge_dir_arr, model->BuildMaterial());
      vtkDiscreteModelFace::SafeDownCast(face)->AddCellsToGeometry(polyId2CellIds[outerPoly]);
      polyId2InnerLoopsAndFaces[outerPoly].second = face;
    }
  }

  //=================Add Innerloops to faces====================//

  std::map<vtkIdType, std::pair<std::list<vtkIdType>, vtkModelFace*> >::iterator polyIter =
    polyId2InnerLoopsAndFaces.begin();
  //Loop over all the faces that have already been created
  for (; polyIter != polyId2InnerLoopsAndFaces.end(); polyIter++)
  {
    vtkModelFace* face = (*polyIter).second.second;
    std::list<vtkIdType>::const_iterator innerLoopIter = (*polyIter).second.first.begin();
    //Add the innerloops to this face
    for (; innerLoopIter != (*polyIter).second.first.end(); innerLoopIter++)
    {
      vtkIdType loopIndex = (*innerLoopIter);
      int numEdges;
      vtkModelEdge** edge_arr;
      int* edge_dir_arr;
      if (!loopId2WalkableLoop[loopIndex].getLoopOrdering(numEdges, edge_arr, edge_dir_arr, false))
      {
        vtkErrorMacro("Error with walking the inner loop");
        return;
      }
      vtkDiscreteModelFace::SafeDownCast(face)->AddLoop(numEdges, edge_arr, edge_dir_arr);
    }
  }

  this->OperateSucceeded = 1;
  ModelWrapper->InitializeWithModelGeometry();
  return;
}
