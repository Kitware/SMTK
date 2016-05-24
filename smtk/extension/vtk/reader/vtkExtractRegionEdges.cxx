//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkExtractRegionEdges.h"

#include "vtkBoundingBox.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCleanPolyData.h"
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkStripper.h"
#include "vtkTimerLog.h"
#include "vtkUnstructuredGrid.h"
#include <map>
#include <deque>
#include <set>

namespace smtk {
  namespace vtk {

vtkStandardNewMacro(vtkExtractRegionEdges);

struct EdgeSegmentInfo
{
  bool AssignedToEdge;
  vtkIdType EndPts[2];
  vtkIdType LineId;
  int RegionIds[2];

  EdgeSegmentInfo()
    {
    this->AssignedToEdge = false;
    this->LineId = -1;
    this->RegionIds[0] = this->RegionIds[1] = -1;
    }
};


typedef std::pair<vtkIdType, vtkIdType> IdTypePair;
typedef std::pair<int, int> IntPair;
typedef std::vector< EdgeSegmentInfo > EdgeSegmentVectorType;
typedef std::deque< vtkIdType > EdgeType;

struct RegionEdgeType
  {
  bool EdgeUsedInLoop;
  EdgeType *Edge;
  bool ReverseDirection;
  double Bounds[6];
  int Index;
  RegionEdgeType *MainEdge;

  RegionEdgeType()
    {
    this->MainEdge = 0;
    this->Index = -1;
    this->EdgeUsedInLoop = false;
    this->ReverseDirection = false;
    }
  };
typedef std::multimap< int, RegionEdgeType* > EdgeMapType;
typedef std::set< IdTypePair > EdgeSegmentVisitSetType;


struct LoopType
  {
  std::deque< RegionEdgeType* > Edges;
  bool OuterLoop;

  LoopType()
    {
    this->OuterLoop = false;
    }
  };

typedef std::multimap< int, LoopType > LoopMapType;

//----------------------------------------------------------------------------
class vtkExtractRegionEdges::vtkInternal
{
public:
  EdgeSegmentVectorType EdgeSegments;
  EdgeMapType EdgeMap;
  LoopMapType LoopMap;
  std::set<int> UniqueRegionIds;

  void BuildRegionEdges(vtkPolyData *linePolyData);
  void BuildRegionLoops( int regionId );

  void AddLoopToFieldData(vtkFieldData *fieldData, vtkStringArray *loopNameArray,
    LoopMapType::const_iterator &loopIter, int loopIndex);

  ~vtkInternal()
    {
    EdgeMapType::iterator edgeIter = this->EdgeMap.begin();
    for (; edgeIter != this->EdgeMap.end(); edgeIter++)
      {
      if (!edgeIter->second->ReverseDirection)
        {
        delete edgeIter->second->Edge;
        }
      }
    }
};

//----------------------------------------------------------------------------
void vtkExtractRegionEdges::vtkInternal::BuildRegionEdges(vtkPolyData *linePolyData)
{
  vtkPoints *points = linePolyData->GetPoints();

  EdgeSegmentVectorType::iterator outerLoopSegmentIter = this->EdgeSegments.begin();
  for (; outerLoopSegmentIter != this->EdgeSegments.end(); outerLoopSegmentIter++)
    {
    if (outerLoopSegmentIter->AssignedToEdge)
      {
      continue;
      }

    EdgeType *edge = new EdgeType;
    edge->push_back( outerLoopSegmentIter->LineId );
    outerLoopSegmentIter->AssignedToEdge = true;
    vtkBoundingBox bbox;
    bbox.AddPoint( points->GetPoint(outerLoopSegmentIter->EndPts[0]) );
    bbox.AddPoint( points->GetPoint(outerLoopSegmentIter->EndPts[1]) );

    // 1st search forward from the end
    vtkIdType currentPtId = outerLoopSegmentIter->EndPts[1];
    vtkIdType currentCellId = outerLoopSegmentIter->LineId;
    vtkIdType *cellIds;
    unsigned short nCells;
    size_t lastSize = 0;
    while(edge->size() != lastSize)
      {
      lastSize = edge->size();
      linePolyData->GetPointCells(currentPtId, nCells, cellIds);
      for (int i = 0; i < nCells; i++)
        {
        if (cellIds[i] == currentCellId || this->EdgeSegments[ cellIds[i] ].AssignedToEdge ||
          outerLoopSegmentIter->RegionIds[0] != this->EdgeSegments[ cellIds[i] ].RegionIds[0] ||
          outerLoopSegmentIter->RegionIds[1] != this->EdgeSegments[ cellIds[i] ].RegionIds[1])
          {
          continue;
          }
        currentPtId = this->EdgeSegments[ cellIds[i] ].EndPts[1];
        currentCellId = cellIds[i];
        this->EdgeSegments[ currentCellId ].AssignedToEdge = true;
        edge->push_back( currentCellId );
        bbox.AddPoint( points->GetPoint(currentPtId) );
        break;
        }
      }

    // now search backward from the start
    lastSize = 0;
    currentPtId = outerLoopSegmentIter->EndPts[0];
    currentCellId = outerLoopSegmentIter->LineId;
    while(edge->size() != lastSize)
      {
      lastSize = edge->size();
      linePolyData->GetPointCells(currentPtId, nCells, cellIds);
      for (int i = 0; i < nCells; i++)
        {
        if (cellIds[i] == currentCellId || this->EdgeSegments[ cellIds[i] ].AssignedToEdge ||
          outerLoopSegmentIter->RegionIds[0] != this->EdgeSegments[ cellIds[i] ].RegionIds[0] ||
          outerLoopSegmentIter->RegionIds[1] != this->EdgeSegments[ cellIds[i] ].RegionIds[1])
          {
          continue;
          }
        currentPtId = this->EdgeSegments[ cellIds[i] ].EndPts[0];
        currentCellId = cellIds[i];
        this->EdgeSegments[ currentCellId ].AssignedToEdge = true;
        edge->push_front( currentCellId );
        bbox.AddPoint( points->GetPoint(currentPtId) );
        break;
        }
      }

    RegionEdgeType *regionEdge = new RegionEdgeType;
    bbox.GetBounds( regionEdge->Bounds );
    regionEdge->Edge = edge;
    this->EdgeMap.insert(
      std::pair< int, RegionEdgeType* >(outerLoopSegmentIter->RegionIds[1], regionEdge) );
    if (outerLoopSegmentIter->RegionIds[0] != -1)
      {
      // the lesser edge, if != -1, needs to be reversed, and doesn't own the edge
      RegionEdgeType *otherRegionEdge = new RegionEdgeType;
      bbox.GetBounds( otherRegionEdge->Bounds );
      otherRegionEdge->Edge = edge;
      otherRegionEdge->ReverseDirection = true;
      otherRegionEdge->MainEdge = regionEdge; // so we can get the index for output
      this->EdgeMap.insert(
        std::pair< int, RegionEdgeType* >(outerLoopSegmentIter->RegionIds[0], otherRegionEdge) );
      }
    }

  EdgeMapType::iterator edgeIter = this->EdgeMap.begin();
  int edgeIndex = 0;
  for (; edgeIter != this->EdgeMap.end(); edgeIter++)
    {
    if (!edgeIter->second->ReverseDirection)
      {
      edgeIter->second->Index = edgeIndex++;
      }
    }
}

//----------------------------------------------------------------------------
void vtkExtractRegionEdges::vtkInternal::BuildRegionLoops(int regionId)
{
  EdgeMapType::iterator regionStart = this->EdgeMap.lower_bound( regionId );
  EdgeMapType::iterator regionEnd = this->EdgeMap.upper_bound( regionId );

  EdgeMapType::iterator outerLoopIter = regionStart;
  int loopsAdded = 0;
  for (; outerLoopIter != regionEnd; outerLoopIter++)
    {
    if (outerLoopIter->second->EdgeUsedInLoop)
      {
      continue;
      }

    LoopType loop;
    loop.Edges.push_back( outerLoopIter->second );
    outerLoopIter->second->EdgeUsedInLoop = true;

    vtkIdType loopStart = loop.Edges.front()->ReverseDirection ?
      this->EdgeSegments[ loop.Edges.front()->Edge->back() ].EndPts[1] :
      this->EdgeSegments[ loop.Edges.front()->Edge->front() ].EndPts[0];
    vtkIdType loopEnd = loop.Edges.back()->ReverseDirection ?
      this->EdgeSegments[ loop.Edges.back()->Edge->front() ].EndPts[0] :
      this->EdgeSegments[ loop.Edges.back()->Edge->back() ].EndPts[1];

    size_t lastSize = 0;
    while (loop.Edges.size() != lastSize && loopStart != loopEnd)
      {
      lastSize = loop.Edges.size();
      EdgeMapType::iterator edgeIter = outerLoopIter;
      for (++edgeIter; edgeIter != regionEnd && loopStart != loopEnd; edgeIter++)
        {
        if (edgeIter->second->EdgeUsedInLoop)
          {
          continue;
          }

        vtkIdType edgeStart = edgeIter->second->ReverseDirection ?
          this->EdgeSegments[ edgeIter->second->Edge->back() ].EndPts[1] :
          this->EdgeSegments[ edgeIter->second->Edge->front() ].EndPts[0];
        vtkIdType edgeEnd = edgeIter->second->ReverseDirection ?
          this->EdgeSegments[ edgeIter->second->Edge->front() ].EndPts[0] :
          this->EdgeSegments[ edgeIter->second->Edge->back() ].EndPts[1];

        // try to add to front of the loop
        if (edgeEnd == loopStart)
          {
          loop.Edges.push_front( edgeIter->second );
          edgeIter->second->EdgeUsedInLoop = true;
          loopStart = edgeStart;
          }
        else if (edgeStart == loopEnd) // and then try the end
          {
          loop.Edges.push_back( edgeIter->second );
          edgeIter->second->EdgeUsedInLoop = true;
          loopEnd = edgeEnd;
          }
        }
      }

    this->LoopMap.insert( std::pair< int, LoopType >(regionId, loop) );
    loopsAdded++;
    }

  // now determine the outer loop
  if (loopsAdded == 0)
    {
    vtkGenericWarningMacro("No loops added for region: " << regionId);
    }
  else if (loopsAdded == 1)
    {
    this->LoopMap.lower_bound( regionId )->second.OuterLoop = true;
    }
  else
    {
    LoopMapType::iterator regionLoopStart = this->LoopMap.lower_bound( regionId );
    LoopMapType::iterator regionLoopEnd = this->LoopMap.upper_bound( regionId );

    LoopMapType::iterator outerLoop;
    LoopMapType::iterator loopIter = regionLoopStart;

    vtkBoundingBox outerBounds;
    for (; loopIter != regionLoopEnd; loopIter++)
      {
      std::deque< RegionEdgeType* >::iterator edgeIter = loopIter->second.Edges.begin();
      vtkBoundingBox bbox;
      for (; edgeIter != loopIter->second.Edges.end(); edgeIter++)
        {
        bbox.AddBounds( (*edgeIter)->Bounds );
        }
      if ( loopIter == regionLoopStart || bbox.Contains(outerBounds) )
        {
        outerBounds = bbox;
        outerLoop = loopIter;
        }
      }
    outerLoop->second.OuterLoop = true;
    }
}


//----------------------------------------------------------------------------
void vtkExtractRegionEdges::vtkInternal::AddLoopToFieldData(vtkFieldData *fieldData,
                                                            vtkStringArray *loopNameArray,
                                                            LoopMapType::const_iterator &loopIter,
                                                            int loopIndex)
{
  char arrayName[32];  // excessively large
  vtkIdTypeArray *loopArray = vtkIdTypeArray::New();
  sprintf(arrayName, "Loop %d", loopIndex);
  loopArray->SetName(arrayName);
  loopNameArray->InsertNextValue(arrayName);
  // region# | OuterLoop flag | edge0 | edge0 direction | edge1 | edge1 | direction
  loopArray->Allocate( loopIter->second.Edges.size() * 2 + 2 );
  fieldData->AddArray( loopArray );
  loopArray->FastDelete();

  loopArray->InsertNextValue( loopIter->first );
  loopArray->InsertNextValue( loopIter->second.OuterLoop );
  std::deque< RegionEdgeType* >::const_iterator edgeIter = loopIter->second.Edges.begin();
  for (; edgeIter != loopIter->second.Edges.end(); edgeIter++)
    {
    int edgeIndex  = (*edgeIter)->Index < 0 ?
      (*edgeIter)->MainEdge->Index : (*edgeIter)->Index;

    loopArray->InsertNextValue( edgeIndex );
    loopArray->InsertNextValue( (*edgeIter)->ReverseDirection ? 0 : 1 );
    }
}

//----------------------------------------------------------------------------
vtkExtractRegionEdges::vtkExtractRegionEdges()
{
  this->Internal = new vtkInternal;
  this->RegionArrayName = 0;
  this->RegionIdentifiersModified = false;
}

//----------------------------------------------------------------------------
vtkExtractRegionEdges::~vtkExtractRegionEdges()
{
  this->SetRegionArrayName(0);
  delete this->Internal;
}

//----------------------------------------------------------------------------
int vtkExtractRegionEdges::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPointSet");
  return 1;
}

//----------------------------------------------------------------------------
int vtkExtractRegionEdges::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPointSet *input = vtkPointSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!this->RegionArrayName || !vtkIntArray::SafeDownCast(
    input->GetCellData()->GetArray(this->RegionArrayName) ))
    {
    vtkErrorMacro("Unable to find region array! Can't extract region edges.");
    output->ShallowCopy( input );
    return 1;
    }

  // if input isn't polydata need to convert to polydata
  vtkSmartPointer<vtkPolyData> tempPolyData = vtkSmartPointer<vtkPolyData>::New();
  if ( !vtkPolyData::SafeDownCast(input) )
    {
    this->ConvertInputToPolyData( input, tempPolyData );
    }
  else
    {
    // we would need to clean up so tempPolyData only polys
    //vtkErrorMacro("Actually don't support vtkPolyData input yet!");
    tempPolyData->ShallowCopy( input );
    }

  tempPolyData->BuildLinks();

  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPolyData> linePolyData = vtkSmartPointer<vtkPolyData>::New();
  this->ExtractRegionEdgeSegments( tempPolyData, lines, linePolyData );

  // initial output setup now that we have lines
  int numberOfLines = lines->GetNumberOfCells();
  int numberOfCells = numberOfLines + tempPolyData->GetNumberOfPolys();
  vtkCellData *inputCD = tempPolyData->GetCellData();
  vtkCellData *outputCD = output->GetCellData();
  outputCD->CopyGlobalIdsOn();
  outputCD->CopyAllocate(inputCD, numberOfCells, numberOfCells / 10);
  output->SetPoints( tempPolyData->GetPoints() );
  output->SetLines( lines );
  output->SetPolys( tempPolyData->GetPolys() );
  output->BuildLinks();
  // copy cell poly data
  int i = numberOfLines;
  for (int j = 0; i < numberOfCells; i++, j++)
    {
    outputCD->CopyData(inputCD, j, i);
    }

  this->UpdateRegionIdentifiersIfNecessary( output );
  this->BuildRegionLoops( linePolyData );

  this->SetupOutputFieldData( output );

  output->DeleteCells(); // delete "cells" and "links" arrays

  return 1;
}

//----------------------------------------------------------------------------
void vtkExtractRegionEdges::ExtractRegionEdgeSegments(vtkPolyData *input,
                                                      vtkCellArray *lines,
                                                      vtkPolyData *linePolyData)
{
  // visit every polygon
  vtkCellArray *polys = input->GetPolys();
  polys->InitTraversal();
  vtkIdType npts, *pts;
  IdTypePair key;
  int regions[2];

  vtkIntArray *regionArray = vtkIntArray::SafeDownCast(
    input->GetCellData()->GetArray(this->RegionArrayName) );
  vtkIdType cellIndex = 0;
  vtkSmartPointer<vtkIdList> neighborCellIds = vtkSmartPointer<vtkIdList>::New();
  // expecting only 1, but per chance there is a non-manifold issue
  neighborCellIds->Allocate(4);
  EdgeSegmentVisitSetType edgeSegmentVisitSet;
  EdgeSegmentInfo edgeSegment;
  for (; polys->GetNextCell(npts, pts); cellIndex++ )
    {
    // loop over the edges
    vtkIdType prevPt = pts[npts - 1];
    for (int i = 0; i < npts; prevPt = pts[i++])
      {
      key = prevPt < pts[i] ?
        std::make_pair(prevPt, pts[i]) : std::make_pair(pts[i], prevPt);
      // have we checked this edge?
      if ( edgeSegmentVisitSet.find( key ) != edgeSegmentVisitSet.end() )
        {
        continue;  // been here (done that)
        }
      edgeSegmentVisitSet.insert(key);

      regions[0] = regionArray->GetValue(cellIndex);

      // determine the membership
      input->GetCellEdgeNeighbors( cellIndex, prevPt, pts[i], neighborCellIds);

      if (neighborCellIds->GetNumberOfIds() < 2)
        {
        if (neighborCellIds->GetNumberOfIds() == 1)
          {
          regions[1] = regionArray->GetValue( neighborCellIds->GetId(0) );
          if (regions[0] == regions[1])
            {
            continue; // not a boundary, don't add it to the EdgeMap
            }
          edgeSegment.RegionIds[0] = regions[0] < regions[1] ? regions[0] : regions[1];
          edgeSegment.RegionIds[1] = regions[0] < regions[1] ? regions[1] : regions[0];
          this->Internal->UniqueRegionIds.insert( regions[0] );
          this->Internal->UniqueRegionIds.insert( regions[1] );
          }
        else // == 0
          {
          edgeSegment.RegionIds[0] = -1;
          edgeSegment.RegionIds[1] = regions[0];
          this->Internal->UniqueRegionIds.insert( regions[0] );
          }
        // always order based on the greater regionId
        if (regions[0] != edgeSegment.RegionIds[1])
          {
          edgeSegment.EndPts[1] = prevPt;
          edgeSegment.EndPts[0] = pts[i];
          }
        else
          {
          edgeSegment.EndPts[0] = prevPt;
          edgeSegment.EndPts[1] = pts[i];
          }
        this->Internal->EdgeSegments.push_back(edgeSegment);
        }
      else // neighborCellIds->GetNumberOfIds() > 1
        {
        vtkWarningMacro("Non-manifold edge");
        }
      }
    }

  // now create lines so we have the line index in our STL structures
  int numberOfLines = static_cast<int>(this->Internal->EdgeSegments.size());
  lines->Allocate( numberOfLines * 3 ); // 2 points per line (and 1 for npts)
  EdgeSegmentVectorType::iterator edgeSegmentIter = this->Internal->EdgeSegments.begin();
  for (; edgeSegmentIter != this->Internal->EdgeSegments.end(); edgeSegmentIter++)
    {
    edgeSegmentIter->LineId =
      lines->InsertNextCell(2, edgeSegmentIter->EndPts);
    }

  linePolyData->SetPoints( input->GetPoints() );
  linePolyData->SetLines( lines );
  linePolyData->BuildLinks();
}

//----------------------------------------------------------------------------
void vtkExtractRegionEdges::BuildRegionLoops(vtkPolyData *linePolyData)
{
  // 1st build edges from the segments
  this->Internal->BuildRegionEdges( linePolyData );

  // then loops from edges
  std::set<int>::const_iterator regionIdIter = this->Internal->UniqueRegionIds.begin();
  for (; regionIdIter != this->Internal->UniqueRegionIds.end(); regionIdIter++)
    {
    this->Internal->BuildRegionLoops(*regionIdIter);
    }
}

//----------------------------------------------------------------------------
void vtkExtractRegionEdges::ConvertInputToPolyData(vtkPointSet *pointSetInput,
                                                   vtkPolyData *polyData)
{
  vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(pointSetInput);

  // for now only handle UnstructuredGrid
  if ( !input )
    {
    vtkErrorMacro("Can only convert vtkUnstructuredGrid input!");
    return;
    }
  polyData->SetPoints( pointSetInput->GetPoints() );
  vtkIdType numCells = input->GetNumberOfCells();

  vtkCellData *inputCD = input->GetCellData();
  vtkCellData *outputCD = polyData->GetCellData();
  outputCD->CopyGlobalIdsOn();
  outputCD->CopyAllocate(inputCD, numCells, numCells/2);

  vtkCellArray *newPolys = vtkCellArray::New();
  polyData->SetPolys( newPolys );
  newPolys->FastDelete();
  newPolys->Allocate( numCells );
  vtkIdType *ids, cellId;
  vtkIdType *cellPointer;
  int cellType, numCellPts;
  unsigned char* cellTypes = input->GetCellTypesArray()->GetPointer(0);

  // initialize the pointer to the cells for fast traversal.
  cellPointer = input->GetCells()->GetPointer();
  vtkIdType numberOfNewCells = 0;
  for(cellId = 0; cellId < numCells; cellId++)
    {
    // Direct access to cells.
    cellType = cellTypes[cellId];
    numCellPts = cellPointer[0];
    ids = cellPointer+1;
    // Move to the next cell.
    cellPointer += (1 + *cellPointer);

    if (cellType == VTK_TRIANGLE || cellType == VTK_QUAD || cellType == VTK_POLYGON)
      {
      newPolys->InsertNextCell(numCellPts, ids);
      outputCD->CopyData(inputCD, cellId, numberOfNewCells++);
      }
    else
      {
      vtkErrorMacro("Unsupported type: " << cellType);
      }
    }
}

//----------------------------------------------------------------------------
void vtkExtractRegionEdges::UpdateRegionIdentifiersIfNecessary(vtkPolyData *outputPD)
{
  // use region edges to update the region ids... so that each loop will be a
  // distinct region
  vtkCellArray *polys = outputPD->GetPolys();

  vtkIntArray *newRegionIds = vtkIntArray::New();
  newRegionIds->Allocate( outputPD->GetNumberOfCells() );
  for (vtkIdType i = 0; i < outputPD->GetNumberOfCells(); i++)
    {
    newRegionIds->InsertNextValue(-1);
    }

  vtkSmartPointer<vtkIdList> neighborIds = vtkSmartPointer<vtkIdList>::New();
  neighborIds->Allocate(2); // at most expecting maybe another poly and maybe a line

  int regionId = 1; // start at 1 because fortran isn't going to like 0
  polys->InitTraversal();
  vtkIdType npts, *pts;
  std::vector<vtkIdType> processStack;
  processStack.reserve( outputPD->GetNumberOfPolys() );
  for (vtkIdType cellIndex = outputPD->GetNumberOfLines();
    cellIndex < outputPD->GetNumberOfCells(); cellIndex++ )
    {
    if (newRegionIds->GetValue( cellIndex ) != -1)
      {
      continue;
      }

    processStack.clear();
    newRegionIds->SetValue( cellIndex, regionId );
    processStack.push_back( cellIndex );
    while (processStack.size())
      {
      vtkIdType currentIndex = processStack.back();
      processStack.pop_back();
      outputPD->GetCellPoints( currentIndex, npts, pts);

      vtkIdType prevPt = pts[npts - 1];
      for (vtkIdType i = 0; i < npts; prevPt = pts[i++])
        {
        outputPD->GetCellEdgeNeighbors(currentIndex, prevPt, pts[i], neighborIds);
        if (neighborIds->GetNumberOfIds() == 0 || neighborIds->GetNumberOfIds() > 2)
          {
          vtkErrorMacro("Shouldn't happen!!!! " << neighborIds->GetNumberOfIds());
          continue; // must be an edge segment or a poly
          }
        if (neighborIds->GetNumberOfIds() > 1 ||
          neighborIds->GetId(0) < outputPD->GetNumberOfLines() ||
          newRegionIds->GetValue( neighborIds->GetId(0) ) > -1)
          {
          // if > 1, must be edge/line and poly... edge stops walk to poly;
          // or it is an edge/line (no poly to walk to)
          // or we've already visited the cell
          continue;
          }

        newRegionIds->SetValue(neighborIds->GetId(0), regionId);
        processStack.push_back( neighborIds->GetId(0) );
        }
      }
    regionId++;
    }

  if (static_cast<size_t>(regionId - 1) != this->Internal->UniqueRegionIds.size())
    {
    this->RegionIdentifiersModified = true;
    vtkCellArray *lines = outputPD->GetLines();
    std::string newArrayName = "Original ";
    newArrayName += this->RegionArrayName;
    newRegionIds->SetName( this->RegionArrayName );
    vtkDataArray *oldRegionIds = outputPD->GetCellData()->GetArray( this->RegionArrayName );
    oldRegionIds->SetName( newArrayName.c_str() );
    outputPD->GetCellData()->AddArray( newRegionIds );
    outputPD->GetCellData()->SetScalars( newRegionIds );

    // update the region for each edge segment
    vtkIdList *neighborCellIds = vtkIdList::New();
    neighborCellIds->Allocate(2); // should be, at most, two polys shared by the edge
    vtkIdType numberOfLines = outputPD->GetNumberOfLines(), tempId;
    for (vtkIdType i = 0; i < numberOfLines; i++)
      {
      outputPD->GetCellEdgeNeighbors( i,
        this->Internal->EdgeSegments[i].EndPts[0],
        this->Internal->EdgeSegments[i].EndPts[1], neighborCellIds);
      if (neighborCellIds->GetNumberOfIds() == 1)
        {
        this->Internal->EdgeSegments[i].RegionIds[1] =
          newRegionIds->GetValue( neighborCellIds->GetId(0) );
        }
      else if (neighborCellIds->GetNumberOfIds() == 2)
        {
        vtkIdType regions[2] = { newRegionIds->GetValue( neighborCellIds->GetId(0) ),
          newRegionIds->GetValue( neighborCellIds->GetId(1) ) };
        vtkIdType oldRegions[2] = {
          static_cast<vtkIdType>(oldRegionIds->GetTuple1( neighborCellIds->GetId(0) )),
          static_cast<vtkIdType>(oldRegionIds->GetTuple1( neighborCellIds->GetId(1) )) };
        this->Internal->EdgeSegments[i].RegionIds[0] =
          regions[0] < regions[1] ? regions[0] : regions[1];
        this->Internal->EdgeSegments[i].RegionIds[1] =
          regions[0] < regions[1] ? regions[1] : regions[0];
        //We need to maintain the requirement that the edge segment should follow
        // the region with the larger ID - the boils down to detecting if the
        // relationship between the regions ID has changed when they were mapped
        // into the new region ID Space
        if (((oldRegions[0] < oldRegions[1]) && (regions[0] > regions[1])) ||
            ((oldRegions[0] > oldRegions[1]) && (regions[0] < regions[1])))
          {
          tempId = this->Internal->EdgeSegments[i].EndPts[0];
          this->Internal->EdgeSegments[i].EndPts[0] =
            this->Internal->EdgeSegments[i].EndPts[1];
          this->Internal->EdgeSegments[i].EndPts[1] = tempId;
          lines->ReverseCell(3*i);
          }
        }
      else
        {
        vtkErrorMacro("Should be either 1 or 2 neighbor cells: " <<
          neighborCellIds->GetNumberOfIds() );
        }
      }

    // make sure UniqueRegionIds is up-to-date
    this->Internal->UniqueRegionIds.clear();
    for (int j = 1; j < regionId; j++)
      {
      this->Internal->UniqueRegionIds.insert( j );
      }

    neighborCellIds->Delete();
    }
  else
    {
    this->RegionIdentifiersModified = false;
    }
  newRegionIds->Delete();
}

//----------------------------------------------------------------------------
void vtkExtractRegionEdges::SetupOutputFieldData(vtkPolyData *output)
{
  // geometry / topology is setup, now setup the field data
  vtkIdTypeArray *vertexArray = vtkIdTypeArray::New();
  vertexArray->SetName("Vertices");
  vertexArray->Allocate( this->Internal->EdgeMap.size() );
  output->GetFieldData()->AddArray( vertexArray );
  vertexArray->FastDelete();

  // loop over all the edges, adding (unique) vertices for end of each edge
  EdgeMapType::const_iterator edgeIter = this->Internal->EdgeMap.begin();
  vtkSmartPointer<vtkIdList> uniqueVertices = vtkSmartPointer<vtkIdList>::New();
  for (; edgeIter != this->Internal->EdgeMap.end(); edgeIter++)
    {
    uniqueVertices->InsertUniqueId( this->Internal->EdgeSegments[
      edgeIter->second->Edge->front() ].EndPts[0] );
    uniqueVertices->InsertUniqueId( this->Internal->EdgeSegments[
      edgeIter->second->Edge->back() ].EndPts[1] );
    }

  // now add the vertices to the vertexArray
  for (vtkIdType i = 0; i < uniqueVertices->GetNumberOfIds(); i++)
    {
    vertexArray->InsertNextValue( uniqueVertices->GetId(i) );
    }
  vertexArray->Squeeze();

  vtkStringArray *edgeNameArray = vtkStringArray::New();
  edgeNameArray->SetName( "Edge Names" );
  output->GetFieldData()->AddArray( edgeNameArray );
  edgeNameArray->FastDelete();

  std::map< IntPair, int > edgeMap;
  // now add an array (of segments) for each edge
  char arrayName[32];  // excessively large
  for (edgeIter = this->Internal->EdgeMap.begin();
    edgeIter != this->Internal->EdgeMap.end(); edgeIter++)
    {
    if ( edgeIter->second->Index < 0 )
      {
      continue;
      }

    vtkIdTypeArray *edgeArray = vtkIdTypeArray::New();
    sprintf(arrayName, "Edge(%d, %d):%d",
      this->Internal->EdgeSegments[ edgeIter->second->Edge->front() ].RegionIds[0],
      this->Internal->EdgeSegments[ edgeIter->second->Edge->front() ].RegionIds[1],
      edgeIter->second->Index);
    edgeArray->SetName(arrayName);
    edgeNameArray->InsertNextValue(arrayName);

    // +2 for the vertex indices (which come 1st)
    edgeArray->Allocate( edgeIter->second->Edge->size() + 2);
    output->GetFieldData()->AddArray( edgeArray );
    edgeArray->FastDelete();

    edgeArray->InsertNextValue( uniqueVertices->IsId(
      this->Internal->EdgeSegments[ edgeIter->second->Edge->front() ].EndPts[0]) );
    edgeArray->InsertNextValue( uniqueVertices->IsId(
      this->Internal->EdgeSegments[ edgeIter->second->Edge->back() ].EndPts[1]) );

    std::deque< vtkIdType >::const_iterator edgeSegmentIter = edgeIter->second->Edge->begin();
    for (; edgeSegmentIter != edgeIter->second->Edge->end(); edgeSegmentIter++)
      {
      edgeArray->InsertNextValue( *edgeSegmentIter );
      }
    }

  vtkStringArray *loopNameArray = vtkStringArray::New();
  loopNameArray->SetName( "Loop Names" );
  output->GetFieldData()->AddArray( loopNameArray );
  loopNameArray->FastDelete();

  // and now (finally) the loops
  std::set<int>::const_iterator regionIdIter = this->Internal->UniqueRegionIds.begin();
  int loopCounter = 0;
  for (; regionIdIter != this->Internal->UniqueRegionIds.end(); regionIdIter++)
    {
    LoopMapType::iterator regionStart = this->Internal->LoopMap.lower_bound( *regionIdIter );
    LoopMapType::iterator regionEnd = this->Internal->LoopMap.upper_bound( *regionIdIter );
    LoopMapType::const_iterator loopIter;
    // 1st find the outer loop and add it
    for (loopIter = regionStart; loopIter != regionEnd; loopIter++)
      {
      if (!loopIter->second.OuterLoop)
        {
        continue;
        }

      this->Internal->AddLoopToFieldData(output->GetFieldData(), loopNameArray,
        loopIter, loopCounter++ );
      break;
      }

    // now any inner loops
    for (loopIter = regionStart; loopIter != regionEnd; loopIter++)
      {
      if (loopIter->second.OuterLoop)
        {
        continue;
        }

      this->Internal->AddLoopToFieldData(output->GetFieldData(), loopNameArray,
        loopIter, loopCounter++ );
      }
    }
}

//----------------------------------------------------------------------------
void vtkExtractRegionEdges::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

  } // namespace vtk
} // namespace smtk
