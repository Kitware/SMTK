//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBPrepareForTriangleMesher.h"

#include "vtkCellArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkFieldData.h"
#include "vtkCellData.h"

vtkStandardNewMacro(vtkCMBPrepareForTriangleMesher);

using namespace CmbFaceMesherClasses;

//-----------------------------------------------------------------------------
//Internal point structure to store unique points in a map
struct InternalPt
  {
  double x,y,z;
  friend bool operator < (const InternalPt& l,const InternalPt& r)
    {
    return l.x != r.x ? (l.x < r.x) : l.y != r.y ? (l.y < r.y) : l.z < r.z;
    }
  InternalPt(double _x, double _y, double _z):x(_x),y(_y),z(_z){};
  };
//-----------------------------------------------------------------------------
vtkCMBPrepareForTriangleMesher::vtkCMBPrepareForTriangleMesher()
  {
  arcArraySize = -1;
  loopArraySize = -1;
  numCells = -1;
  mapInfoInitialized = false;
  numLoopsAdded = 0;
  numArcsAdded = 0;
  numNodesAdded = 0;

  fieldCellArrayOffset = 0;
  fieldCellArraySize = 0;
  fieldArcId = 0;
  fieldLoop1 = 0;
  fieldLoop2 = 0;
  fieldEndpoint1 = 0;
  fieldEndpoint2 = 0;
  fieldLoopInfo = 0;

  cellElementIds = 0;
  }
//-----------------------------------------------------------------------------
//
void vtkCMBPrepareForTriangleMesher::InitializeNewMapInfo()
  {
  numLoopsAdded = 0;
  numArcsAdded = 0;

  fieldCellArrayOffset = vtkIdTypeArray::New();
  fieldCellArrayOffset->SetName("CellArrayOffset");
  fieldCellArrayOffset->SetNumberOfComponents(1);
  fieldCellArraySize = vtkIdTypeArray::New();
  fieldCellArraySize->SetName("CellArraySize");
  fieldCellArraySize->SetNumberOfComponents(1);
  fieldArcId = vtkIdTypeArray::New();
  fieldArcId->SetName("ArcId");
  fieldArcId->SetNumberOfComponents(1);
  fieldLoop1 = vtkIdTypeArray::New();
  fieldLoop1->SetName("Loop1");
  fieldLoop1->SetNumberOfComponents(1);
  fieldLoop2 = vtkIdTypeArray::New();
  fieldLoop2->SetName("Loop2");
  fieldLoop2->SetNumberOfComponents(1);
  fieldEndpoint1 = vtkIdTypeArray::New();
  fieldEndpoint1->SetName("ArcEndpoint1");
  fieldEndpoint1->SetNumberOfComponents(1);
  fieldEndpoint2 = vtkIdTypeArray::New();
  fieldEndpoint2->SetName("ArcEndpoint2");
  fieldEndpoint2->SetNumberOfComponents(1);

  fieldLoopInfo = vtkIdTypeArray::New();
  fieldLoopInfo->SetName("LoopInfo");
  fieldLoopInfo->SetNumberOfComponents(2);

  cellElementIds = vtkIdTypeArray::New();
  cellElementIds->SetName("ElementIds");
  cellElementIds->SetNumberOfComponents(1);
  //If we know the array size set the number of tuples
  if(this->arcArraySize != -1)
    {
    fieldCellArrayOffset->SetNumberOfTuples(arcArraySize);
    fieldCellArraySize->SetNumberOfTuples(arcArraySize);
    fieldArcId->SetNumberOfTuples(arcArraySize);
    fieldLoop1->SetNumberOfTuples(arcArraySize);
    fieldLoop2->SetNumberOfTuples(arcArraySize);
    fieldEndpoint1->SetNumberOfTuples(arcArraySize);
    fieldEndpoint2->SetNumberOfTuples(arcArraySize);
    }
  if(this->loopArraySize != -1)
    {
    fieldLoopInfo->SetNumberOfTuples(loopArraySize);
    }
  if(this->numCells != -1)
    {
    cellElementIds->SetNumberOfTuples(numCells);
    }
  mapInfoInitialized = true;
  }
//-----------------------------------------------------------------------------
void vtkCMBPrepareForTriangleMesher::SetNumberOfArcs(const vtkIdType& num)
{
  if(mapInfoInitialized)
    {
    vtkErrorMacro("Field data has already been created.\
        Set the number of arcs before initializing new map field data");
    }
  arcArraySize=num;
}
//-----------------------------------------------------------------------------
void vtkCMBPrepareForTriangleMesher::SetNumberOfLoops(const vtkIdType& num)
{
  if(mapInfoInitialized)
    {
    vtkErrorMacro("MapInfohas already been created.\
        Set the number of loops before initializing new map field/cell data");
    }
  loopArraySize=num;
}
//-----------------------------------------------------------------------------
void vtkCMBPrepareForTriangleMesher::SetNumberOfCells(const vtkIdType& num)
  {
  if(mapInfoInitialized)
    {
    vtkErrorMacro("MapInfo data has already been created.\
        Set the number of loops before initializing new map field/cell data");
    }
  this->numCells = num;
  }
//-----------------------------------------------------------------------------
//Sets polydata to query for information or set up new field data
void vtkCMBPrepareForTriangleMesher::SetPolyData(vtkPolyData*  pd)
{
  this->PolyData = pd;
}
//-----------------------------------------------------------------------------
vtkCMBPrepareForTriangleMesher::~vtkCMBPrepareForTriangleMesher()
{
  if(fieldCellArrayOffset)
    {
    fieldCellArrayOffset->Delete();
    }
  if(fieldCellArraySize)
    {
    fieldCellArraySize->Delete();
    }
  if(fieldArcId)
    {
    fieldArcId->Delete();
    }
  if(fieldEndpoint1)
    {
    fieldEndpoint1->Delete();
    }
  if(fieldEndpoint2)
    {
    fieldEndpoint2->Delete();
    }
  if(fieldLoop1)
    {
    fieldLoop1->Delete();
    }
  if(fieldLoop2)
    {
    fieldLoop2->Delete();
    }
  if(fieldLoopInfo)
    {
    fieldLoopInfo->Delete();
    }
  if(cellElementIds)
    {
    cellElementIds->Delete();
    }
}
//-----------------------------------------------------------------------------
//Adds a node id to a vtkvertex
//the order in which you created the vtkvertex must be the same in which you
//add the nodes
vtkIdType vtkCMBPrepareForTriangleMesher::AddNode(const vtkIdType& nodeId)
  {
  if(this->numCells == -1)
    {
    this->cellElementIds->InsertTuple1(numNodesAdded,nodeId);
    }
  else
    {
    this->cellElementIds->SetTuple1(numNodesAdded,nodeId);
    }
  return numNodesAdded++;
  }
//-----------------------------------------------------------------------------
//Creates the necessary field data for an arc
vtkIdType vtkCMBPrepareForTriangleMesher::AddArc(const vtkIdType& CellArrayOffset,
                                const vtkIdType& CellArraySize,
                                const vtkIdType& ArcId,
                                const vtkIdType& Loop1,
                                const vtkIdType& Loop2,
                                const vtkIdType& Endpoint1,
                                const vtkIdType& Endpoint2)
{
  if(this->arcArraySize == -1)
    {
    this->fieldCellArrayOffset->InsertTuple1(numArcsAdded, CellArrayOffset);
    this->fieldCellArraySize->InsertTuple1(numArcsAdded, CellArraySize);
    this->fieldArcId->InsertTuple1(numArcsAdded, ArcId);
    this->fieldLoop1->InsertTuple1(numArcsAdded, Loop1);
    this->fieldLoop2->InsertTuple1(numArcsAdded, Loop2);
    this->fieldEndpoint1->InsertTuple1(numArcsAdded, Endpoint1);
    this->fieldEndpoint2->InsertTuple1(numArcsAdded, Endpoint2);
    }
  else
    {
    this->fieldCellArrayOffset->SetTuple1(numArcsAdded, CellArrayOffset);
    this->fieldCellArraySize->SetTuple1(numArcsAdded, CellArraySize);
    this->fieldArcId->SetTuple1(numArcsAdded, ArcId);
    this->fieldLoop1->SetTuple1(numArcsAdded, Loop1);
    this->fieldLoop2->SetTuple1(numArcsAdded, Loop2);
    this->fieldEndpoint1->SetTuple1(numArcsAdded, Endpoint1);
    this->fieldEndpoint2->SetTuple1(numArcsAdded, Endpoint2);
    }

  //Add element ids as cell data
  //Only will work with VTK_LINES
  vtkIdType startCell = CellArrayOffset/3;
  vtkIdType numCellsInArc = CellArraySize/3;
  for(vtkIdType i = 0; i < numCellsInArc; i++)
    {
    //Insert the cell data after verts
    if(this->numCells == -1)
      {
      this->cellElementIds->InsertTuple1(numNodesAdded+startCell+i,ArcId);
      }
    else
      {
      this->cellElementIds->SetTuple1(numNodesAdded+startCell+i,ArcId);
      }
    }

  //Return the arc index
  return numArcsAdded++;
}
//-----------------------------------------------------------------------------
vtkIdType vtkCMBPrepareForTriangleMesher::AddLoopWithArcs(const vtkIdType& PolyId,
                                              const bool& isOuter,
                                              const std::vector<vtkIdType>& arcIndexes)
  {

  //Adds a loop and then finds the arcs that this loop belongs to and
  //updates their fieldLoop1 and fieldLoop2 field data arrays

  //if this is a new polygon that hasn't been seen before
  vtkIdType loopIndex = -2;
  if(this->loop2loopIndex.find(arcIndexes) == this->loop2loopIndex.end())
    {
    //we only know either the outer or inner loop use at this time
    //put -1 in as a placeholder for now
    loopIndex = isOuter ? this->AddLoop(PolyId,-1) : this->AddLoop(-1,PolyId);
    this->loop2loopIndex[arcIndexes] = loopIndex;
    }
  else
    {
    //we have seen this loop before grab the id so we can update it
    loopIndex = this->loop2loopIndex[arcIndexes];
    double *tup = this->fieldLoopInfo->GetTuple2(loopIndex);
    //Check to make sure there is a place to put the data
    if( !((tup[0] == -1) ^ (tup[1] == -1))) //xor
      {
      vtkErrorMacro("ERROR: loop "<< loopIndex << " seen before but ids are not set correctly. Would be poly " << PolyId);
      }
    else
      {
      //set tup[0] for outer and tup[1] for inner
      tup[!isOuter] = PolyId;
      fieldLoopInfo->SetTuple2(loopIndex,tup[0],tup[1]);
      }
    }

  //Relate the loop back to the arcs
  std::vector<vtkIdType>::const_iterator arcIndexIter = arcIndexes.begin();
  for(;arcIndexIter != arcIndexes.end(); arcIndexIter++)
    {
    vtkIdType arcIndex = (*arcIndexIter);
    //Make sure the arcIndex is valid
    if(arcIndex >= numArcsAdded)
      {
      vtkErrorMacro("Error cannot create a polygon using arc index " << arcIndex << " because there that arc has not been added yet");
      }
    //Get loops the arc is already associated with
    double loop1 = this->fieldLoop1->GetTuple1(arcIndex);
    double loop2 = this->fieldLoop2->GetTuple1(arcIndex);
    //Figure out which field to place the loopIndex
    //only place in the first slot if it is an outer loop
    if ( loop1 == -1 && isOuter)
      {
      this->fieldLoop1->SetTuple1(arcIndex,loopIndex);
      }
    else if( loop2 == -1 )
      {
      this->fieldLoop2->SetTuple1(arcIndex,loopIndex);
      }
    else
      {
      vtkErrorMacro("ERROR: arcIndexes is an invalid loop list");
      }
    }
  return loopIndex;
  }
//-----------------------------------------------------------------------------
vtkIdType vtkCMBPrepareForTriangleMesher::AddLoop(const vtkIdType& OuterPolyId,
                                      const vtkIdType& InnerPolyId )
{
  if(this->loopArraySize == -1)
    {
    this->fieldLoopInfo->InsertTuple2(numLoopsAdded,OuterPolyId,InnerPolyId);
    }
  else
    {
    this->fieldLoopInfo->SetTuple2(numLoopsAdded, OuterPolyId,InnerPolyId);
    }
  numLoopsAdded++;
  //Return the loopId
  return numLoopsAdded-1;
}
//-----------------------------------------------------------------------------
void vtkCMBPrepareForTriangleMesher::FinalizeNewMapInfo()
{
  //If one exists then they all should exist
  if(fieldCellArrayOffset)
    {
    this->PolyData->GetFieldData()->AddArray(fieldCellArrayOffset);
    this->PolyData->GetFieldData()->AddArray(fieldCellArraySize);
    this->PolyData->GetFieldData()->AddArray(fieldArcId);
    this->PolyData->GetFieldData()->AddArray(fieldEndpoint1);
    this->PolyData->GetFieldData()->AddArray(fieldEndpoint2);
    this->PolyData->GetFieldData()->AddArray(fieldLoop1);
    this->PolyData->GetFieldData()->AddArray(fieldLoop2);
    this->PolyData->GetFieldData()->AddArray(fieldLoopInfo);
    this->PolyData->GetCellData()->AddArray(cellElementIds);

    //quickly delete the arrays and make sure that the destructor does not
    //delete them a second time by setting the pointers to 0
    fieldCellArrayOffset->FastDelete();
    fieldCellArraySize->FastDelete();
    fieldArcId->FastDelete();
    fieldEndpoint1->FastDelete();
    fieldEndpoint2->FastDelete();
    fieldLoop1->FastDelete();
    fieldLoop2->FastDelete();
    fieldLoopInfo->FastDelete();
    cellElementIds->FastDelete();
    fieldCellArrayOffset = NULL;
    fieldCellArraySize = NULL;
    fieldArcId = NULL;
    fieldEndpoint1 = NULL;
    fieldEndpoint2 = NULL;
    fieldLoop1 = NULL;
    fieldLoop2 = NULL;
    fieldLoopInfo = NULL;
    cellElementIds = NULL;
    }
  else
    {
    vtkWarningMacro("No field data to finalize");
    }
}
//-----------------------------------------------------------------------------
// Returns an arc contour as polydata
// toReturn must be an initialized vtkPolyData will be returned as the
// requested contour
void vtkCMBPrepareForTriangleMesher::GetArc(vtkIdType requestedArcId, vtkPolyData* toReturn)
{
  //The input polygon must be in the map format
  vtkFieldData* fieldData = this->PolyData->GetFieldData();
  vtkIdTypeArray* arcIds = vtkIdTypeArray::SafeDownCast(fieldData->GetArray("ArcId"));
  vtkIdType arcIndex = 0;

  //Find the correct index into the field data arrays
  bool foundId = false;
  for(; arcIndex < arcIds->GetNumberOfTuples(); arcIndex++)
    {
    if ( arcIds->GetTuple1(arcIndex) == requestedArcId )
      {
      foundId=true;
      break;
      }
    }
  if(!foundId)
    {
    vtkWarningMacro("A valid arc id was not selected");
    }

  //Use index to get arc properties
  vtkIdType cellStart = fieldData->GetArray("CellArrayOffset")->GetTuple1(arcIndex);
  vtkIdType cellArraySize = fieldData->GetArray("CellArraySize")->GetTuple1(arcIndex);

  //Get input info
  vtkCellArray* lines = this->PolyData->GetLines();
  //Create output info
  vtkCellArray* linesToAdd = vtkCellArray::New();
  vtkPoints* pointsToAdd = vtkPoints::New();
  //Allocate an exepcted number of lines and points
  linesToAdd->Allocate(cellArraySize);
  pointsToAdd->SetNumberOfPoints((2*(cellArraySize))/3);

  double p[3]; //temporary point
  vtkIdType lineToInsert[2] = {-1, -1};
  std::map<InternalPt,vtkIdType> pt2ptId; //map to make sure no duplicate inserts
  vtkIdType insertAt = 0;

  //Traverse all cells in arc and add them to output
  if (lines)
    {
    vtkIdType *pts,npts;
    lines->SetTraversalLocation(cellStart);
    while(lines->GetTraversalLocation() < (cellArraySize + cellStart) && /*Check The lines are within bounds*/
          lines->GetNextCell(npts,pts)/*Check There are lines*/ )
      {
      for(vtkIdType j = 0; j < npts; j++)
        {
        //Grab the points this edge uses
        this->PolyData->GetPoint(pts[j],p);
        InternalPt edgePoint(p[0],p[1],p[2]);
        //Check and see if point has already been seen
        std::map<InternalPt,vtkIdType>::iterator foundPt = pt2ptId.find(edgePoint);
        if(foundPt == pt2ptId.end())
          {
          //Point has not been seen
          pointsToAdd->SetPoint(insertAt,p);
          foundPt = pt2ptId.insert(std::pair<InternalPt,vtkIdType>(edgePoint,insertAt++)).first;
          }
        std::swap(lineToInsert[0],lineToInsert[1]);
        lineToInsert[1] = foundPt->second;
        if(j > 0)
          {
          //Insert the previous two points as a line
          linesToAdd->InsertNextCell(2,lineToInsert);
          }
        }
      }
    }
  //Remove any extra allocated space
  pointsToAdd->SetNumberOfPoints(insertAt);
  pointsToAdd->Squeeze();
  linesToAdd->Squeeze();

  //Set output properties
  toReturn->SetPoints(pointsToAdd);
  toReturn->SetLines(linesToAdd);
  pointsToAdd->FastDelete();
  linesToAdd->FastDelete();
}

//-----------------------------------------------------------------------------
bool vtkCMBPrepareForTriangleMesher::IsValidForReading()
{
  vtkFieldData* fieldData;
  bool hasValidFields =false;
  if( this->PolyData && (fieldData=this->PolyData->GetFieldData()) )
  {
  hasValidFields = fieldData->HasArray("Loop1") != 0;
  hasValidFields = hasValidFields && (fieldData->HasArray("Loop2") != 0);
  hasValidFields = hasValidFields && (fieldData->HasArray("LoopInfo") != 0);
  hasValidFields = hasValidFields && (fieldData->HasArray("ArcEndpoint1") != 0);
  hasValidFields = hasValidFields && (fieldData->HasArray("ArcEndpoint2") != 0);
  hasValidFields = hasValidFields && (fieldData->HasArray("ArcId") != 0);
  hasValidFields = hasValidFields && (fieldData->HasArray("CellArrayOffset") != 0);
  hasValidFields = hasValidFields && (fieldData->HasArray("CellArraySize") != 0);
  hasValidFields = hasValidFields && (fieldData->HasArray("Loop1") != 0);
  }
  return hasValidFields;
}
//-----------------------------------------------------------------------------

bool vtkCMBPrepareForTriangleMesher::GetPolyId2ModelFaceRepMap(
    std::map<vtkIdType, ModelFaceRep* >& pid2Face)
{
  // loop index to index into arc field data
  std::map<vtkIdType, std::vector<vtkIdType> > loopId2ArcIndex;

  bool valid = this->IsValidForReading();
  valid = valid && this->BuildLoopId2ArcIndexMap(loopId2ArcIndex);
  valid = valid && this->BuildPolygonId2ModelFaceMap(loopId2ArcIndex, pid2Face);
  return valid;
}
//-----------------------------------------------------------------------------
//Create a mapping from loop ids to a vector of arc indices.
//loop ids are simply the index of the loop in the LoopInfo
//field data array. The field data arrays loop1 and loop2
//store the ids that are used to index into the LoopInfo
//array
bool vtkCMBPrepareForTriangleMesher::BuildLoopId2ArcIndexMap(
    std::map<vtkIdType, std::vector<vtkIdType> >& loopId2ArcIndex)
{
  vtkFieldData* fData = this->PolyData->GetFieldData();
  vtkIdTypeArray* fLoop1 = vtkIdTypeArray::SafeDownCast(fData->GetArray("Loop1"));
  vtkIdTypeArray* fLoop2 = vtkIdTypeArray::SafeDownCast(fData->GetArray("Loop2"));

  typedef std::map<vtkIdType, std::vector<vtkIdType> >::iterator LoopIterator;
  typedef std::pair<LoopIterator,bool> LoopIteratorResult;

  for(int i = 0; i < fLoop1->GetNumberOfTuples(); i++)
    {
    vtkIdType loop1 = fLoop1->GetTuple1(i);
    vtkIdType loop2 = fLoop2->GetTuple1(i);
    //If there is a loop this arc is a part of
    if(loop1 != -1)
      {
      LoopIterator it = loopId2ArcIndex.find(loop1);
      if(it == loopId2ArcIndex.end())
        {
        //New loop seen create a vector to store the arcs associate with this loop
        std::pair<std::map<vtkIdType, std::vector<vtkIdType> >::iterator,bool> pr = loopId2ArcIndex.insert(
          std::pair<vtkIdType, std::vector<vtkIdType> >(loop1,std::vector<vtkIdType>()));
        it = pr.first;
        (*it).second.reserve(fLoop1->GetNumberOfTuples());
        }
      //Add an arc to this loop that has already been seen
      (*it).second.push_back(i);
      }
    //If there is a second loop this arc is a part of
    if(loop1 != loop2 && loop2 != -1)
      {
      std::map<vtkIdType, std::vector<vtkIdType> >::iterator it = loopId2ArcIndex.find(loop2);
      if(it == loopId2ArcIndex.end())
        {
        //New loop seen create a vector to store the arcs associate with this loop
        std::pair<std::map<vtkIdType, std::vector<vtkIdType> >::iterator,bool> pr = loopId2ArcIndex.insert(
          std::pair<vtkIdType, std::vector<vtkIdType> >(loop2,std::vector<vtkIdType>()));
        it = pr.first;
        (*it).second.reserve(fLoop1->GetNumberOfTuples());
        }
      //Add an arc to this loop that has already been seen
      (*it).second.push_back(i);
      }
    }
  //Resize vectors of arcs to be the actual size
  std::map<vtkIdType, std::vector<vtkIdType> >::iterator it = loopId2ArcIndex.begin();
  for (; it != loopId2ArcIndex.end(); it++)
    {
    (*it).second.resize((*it).second.size());
    }

  return true;
  }
//--------------------------------------------------------------------
bool vtkCMBPrepareForTriangleMesher::BuildPolygonId2ModelFaceMap(
    const std::map<vtkIdType, std::vector<vtkIdType> >& loopId2ArcIndex,
    std::map<vtkIdType, ModelFaceRep* >& pid2Face)
{
  //TODO: meshEdgeReps can be created more than once, its not a huge problem
  // because the data just gets copied multiple times from them anyway
  //But it may be a slight improvement to save edges once you create them for later use

  //Check to see if there is node information
  std::map<vtkIdType, std::pair<double,double> > nodeId2point;
  if(this->PolyData->GetCellData()->HasArray("ElementIds"))
    {
    vtkIdTypeArray* elementIds = vtkIdTypeArray::SafeDownCast(
                        this->PolyData->GetCellData()->GetArray("ElementIds"));
    const vtkIdType size = this->PolyData->GetNumberOfCells();
    for(int i = 0; i < size; i++)
      {
      vtkCell* cell = this->PolyData->GetCell(i);
      if(cell->GetCellType() == VTK_VERTEX)
        {
        double pt[3];
        cell->GetPoints()->GetPoint(0,pt);
        vtkIdType nodeId = elementIds->GetTuple1(i);
        nodeId2point[nodeId] = std::make_pair(pt[0],pt[1]);
        }
      else
        {
        break; // no more vertex data is available
        }
      }
    }

  vtkFieldData* fieldData = this->PolyData->GetFieldData();
  vtkIdTypeArray* fLoopInfo  = vtkIdTypeArray::SafeDownCast(
                                        fieldData->GetArray("LoopInfo"));
  vtkIdTypeArray* fEndpoint1 = vtkIdTypeArray::SafeDownCast(
                                        fieldData->GetArray("ArcEndpoint1"));
  vtkIdTypeArray* fEndpoint2 = vtkIdTypeArray::SafeDownCast(
                                        fieldData->GetArray("ArcEndpoint2"));
  vtkIdTypeArray* fArcId     = vtkIdTypeArray::SafeDownCast(
                                        fieldData->GetArray("ArcId"));
  vtkIdTypeArray* fCellOffset= vtkIdTypeArray::SafeDownCast(
                                        fieldData->GetArray("CellArrayOffset"));
  vtkIdTypeArray* fArraySize = vtkIdTypeArray::SafeDownCast(
                                        fieldData->GetArray("CellArraySize"));

  if(!fEndpoint1 || !fEndpoint2 || !fArcId || !fCellOffset ||
     !fArraySize || !fLoopInfo)
    {
    //no point going any fargher no loop info
    return false;
    }

  //Populate the ModelFaceRep's in pid2Face
  for(int loopId = 0; loopId < fLoopInfo->GetNumberOfTuples(); loopId++)
    {
    double *polyId = fLoopInfo->GetTuple2(loopId);
    //iterate twice: loopType = 0 is the outer loop
    //               loopType = 1 is the inner loop
    for(int loopType = 0; loopType < 2; loopType++)
      {
      // -1 and 0 are invalid ids, skip them
      if(polyId[loopType] < 1) continue;

      std::map<vtkIdType, ModelFaceRep* >::iterator currFace =
          pid2Face.find(polyId[loopType]);
      if(currFace == pid2Face.end())
        {
        //Create a new face for a polygon id we haven't seen
        std::pair< std::map<vtkIdType, ModelFaceRep* >::iterator, bool> pr =
          pid2Face.insert( std::pair<vtkIdType, ModelFaceRep*>(
                             polyId[loopType], new ModelFaceRep()));
        currFace = pr.first;
        }

      ModelLoopRep loop(loopId,loopType==1);
      //iterate over each arc index in the loop
      std::map<vtkIdType, std::vector<vtkIdType> >::const_iterator loopVector =
          loopId2ArcIndex.find(loopId);
      std::vector<vtkIdType>::const_iterator loopIter =
          loopVector->second.begin();

      for(;loopIter != loopVector->second.end(); loopIter++)
        {
        vtkIdType arcIndex = (*loopIter);

        vtkIdType arcId = fArcId->GetTuple1(arcIndex);
        ModelEdgeRep edge(arcId);
        //Add the points for this arc by specifying the range in the
        //cell data where this arc exists
        vtkIdType cellOffset = fCellOffset->GetTuple1(arcIndex);
        vtkIdType cellSize = fArraySize->GetTuple1(arcIndex);
        edge.setMeshPoints(this->PolyData,cellOffset,cellSize);

        //If there is node information add it
        if(nodeId2point.size() > 0)
          {
          vtkIdType nodeId1 = fEndpoint1->GetTuple1(arcIndex);
          vtkIdType nodeId2 = fEndpoint2->GetTuple1(arcIndex);
          std::pair<double,double> nodePt1_pair = nodeId2point[nodeId1];
          std::pair<double,double> nodePt2_pair = nodeId2point[nodeId2];
          double nodePt1[2] = {nodePt1_pair.first, nodePt1_pair.second};
          double nodePt2[2] = {nodePt2_pair.first, nodePt2_pair.second};
          edge.addModelVert(nodeId1,nodePt1);
          edge.addModelVert(nodeId2,nodePt2);
          }
        //add the newly created edge to the loop
        loop.addEdge(edge);
        }
      //add the newly created loop to the face
      currFace->second->addLoop(loop);
      }
    }

  return true;
}
//-----------------------------------------------------------------------------
void vtkCMBPrepareForTriangleMesher::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  //TODO: add more to this
  os << "vtkCMBPrepareForTriangleMesher\n";
}
