//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCompleteShells.h"

#include "ModelParserHelper.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellLocator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkSmartPointer.h"

#include <map>
#include <set>
#include <vector>

struct vtkCompleteShellsInternals
{
  std::map<int, std::pair<int, int> > FaceToRegionsMap;
};


vtkStandardNewMacro(vtkCompleteShells);

//----------------------------------------------------------------------------
vtkCompleteShells::vtkCompleteShells()
{
  this->ModelFaceArrayName = 0;
  this->ModelRegionArrayName = 0;
  this->DetectAndFixSubmergedSolids = true;
  this->MinimumSubmergedVoteCountToAvoidWarning = 6;
  this->Internals = new vtkCompleteShellsInternals;
}

//----------------------------------------------------------------------------
vtkCompleteShells::~vtkCompleteShells()
{
  this->SetModelFaceArrayName(0);
  this->SetModelRegionArrayName(0);
  delete this->Internals;
}

//----------------------------------------------------------------------------
int vtkCompleteShells::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
  return 1;
}

//----------------------------------------------------------------------------
int vtkCompleteShells::RequestData(
  vtkInformation * /*request*/,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkPolyData *input = vtkPolyData::GetData(inputVector[0], 0);
  vtkPolyData *output = vtkPolyData::GetData(outputVector, 0);
  if (!input)
    {
    vtkErrorMacro("Input not specified!");
    return 0;
    }

  vtkIntArray* regionArray = vtkIntArray::SafeDownCast(
    input->GetCellData()->GetArray(this->ModelRegionArrayName ?
    this->ModelRegionArrayName : ModelParserHelper::GetShellTagName()) );
  if(regionArray == 0)
    {
    vtkErrorMacro("Could not find region array.");
    return 0;
    }

  vtkDataArray *cellNormals = input->GetCellData()->GetNormals();
  if (!cellNormals)
    {
    vtkErrorMacro("Cell Normals MUST be specifed on the input (probably via runing vtkMasterPolyDataNormals)");
    return 0;
    }

  // whatever we have on the input, copy to the output
  output->ShallowCopy(input);

  vtkDataArray *genericModelFaceArray =
    input->GetCellData()->GetArray(this->ModelFaceArrayName ?
    this->ModelFaceArrayName : ModelParserHelper::GetModelFaceTagName());
  vtkSmartPointer<vtkIdTypeArray> modelFaceArray;
  if (genericModelFaceArray)
    {
    modelFaceArray = vtkIdTypeArray::SafeDownCast( genericModelFaceArray );
    if (!modelFaceArray)
      {
      modelFaceArray = vtkSmartPointer<vtkIdTypeArray>::New();
      modelFaceArray->SetName(ModelParserHelper::GetModelFaceTagName());
      modelFaceArray->DeepCopy( genericModelFaceArray );
      output->GetCellData()->AddArray( modelFaceArray );
      }
    }
  if (!modelFaceArray)
    {
    modelFaceArray = vtkSmartPointer<vtkIdTypeArray>::New();
    modelFaceArray->Allocate(input->GetNumberOfCells());
    modelFaceArray->SetName(ModelParserHelper::GetModelFaceTagName());
    for (int i = 0; i < input->GetNumberOfCells(); i++)
      {
      modelFaceArray->SetValue(i, -1);
      // RSB: instead of setting to -1 thinking we initialize to Region...
      // but not just yet... let's get initial case (valid model faces
      // passed in) working 1st
      }
    }

  // if our modelFaceArray has different name than expected by the builder,
  // need to copy to array that has expected name (array must have come
  // from the input and been specfied via this->ModelFaceArrayName)
  if (strcmp(modelFaceArray->GetName(),ModelParserHelper::GetModelFaceTagName()))
    {
    vtkSmartPointer<vtkIdTypeArray> tmpModelFaceArray =
      vtkSmartPointer<vtkIdTypeArray>::New();
    tmpModelFaceArray->SetName(ModelParserHelper::GetModelFaceTagName());
    tmpModelFaceArray->DeepCopy(modelFaceArray);
    modelFaceArray = tmpModelFaceArray;
    }

  input->BuildLinks();


//

  //visit every polygon... mark polygons that are visited (as visited)


  //char Visited

  // don't care about anything other than polys...
  // assumes we don't have tristrips
  //vtkCellArray *polys = input->GetPolys();
  vtkIdType numCells = input->GetNumberOfCells();

  char *visited = new char [numCells];
  memset(visited, 0, numCells);
  this->NextModelFaceId = 0;


  // create map... faceid to region pair; at this point, one side of every
  // face will be -1;  also, a map is probably overkill, but se just in case
  // faceIds aren't consecutive integers starting at 0.

  std::pair<int, int> regions(-1, -1);
  this->Internals->FaceToRegionsMap.clear();

  vtkIdType cellIndex;
  for (cellIndex = 0; cellIndex < numCells; cellIndex++)
    {
    if (this->Internals->FaceToRegionsMap.find(modelFaceArray->GetValue(cellIndex)) ==
      this->Internals->FaceToRegionsMap.end())
      {
      regions.first = regionArray->GetValue(cellIndex);
      this->Internals->FaceToRegionsMap[modelFaceArray->GetValue(cellIndex)] = regions;
      }
    }
  for (cellIndex = 0; cellIndex < numCells; cellIndex++)
    {
    if (visited[cellIndex])
      {
      continue;
      }

    this->FindRegionEdge(input, cellNormals, regionArray, modelFaceArray,
      cellIndex, visited);
    }

  delete [] visited;

  // do we have any possibly submerged solids?  Only check those faces that
  // have only a single region assigned and that are complete (region = face)
  if ( this->DetectAndFixSubmergedSolids )
    {
    // step through the face to region map, keeping track of all faces that
    // have only a single region assigned (and region used by only one face)
    std::map<int, int> singleFaceRegions;  // first is region, second is face
    std::set<int> multiFaceRegions;
    std::map<int, std::pair<int,int> >::iterator faceMapIter;
    for(faceMapIter = this->Internals->FaceToRegionsMap.begin();
      faceMapIter != this->Internals->FaceToRegionsMap.end(); faceMapIter++)
      {
      // if face is used by two regions, then not interested in either of the regions
      if ( faceMapIter->second.second != -1 )
        {
        // may already be in the set, but no harm to add if already there
        multiFaceRegions.insert( faceMapIter->second.first );
        multiFaceRegions.insert( faceMapIter->second.second );
        }
      else // faceMapIter->second.second == -1
        {
        if ( singleFaceRegions.find(faceMapIter->second.first) != singleFaceRegions.end() )
          {
          // we've already seen this region, so move it to the multiFace map
          multiFaceRegions.insert( faceMapIter->second.first );
          singleFaceRegions.erase( faceMapIter->second.first );
          }
        else if ( multiFaceRegions.count( faceMapIter->second.first ) == 0 )
          {
          // not in either of our containers, so add it to the singleFaceRegions map
          singleFaceRegions[ faceMapIter->second.first ] = faceMapIter->first;
          }
        }
      }

    if ( singleFaceRegions.size() > 0 )
      {
      vtkSmartPointer<vtkCellLocator> cellLocator = vtkSmartPointer<vtkCellLocator>::New();
      cellLocator->SetDataSet( input );
      cellLocator->BuildLocator();

       // now, iterate over each of the regions, checking to see what (if any) the
       // closest enclosing region is
      std::map<int, int>::iterator regionIter;
      for (regionIter = singleFaceRegions.begin(); regionIter != singleFaceRegions.end(); regionIter++)
        {
        this->FindClosestEnclosingRegion( regionIter->first, regionIter->second,
          input, cellNormals, regionArray, modelFaceArray, cellLocator );
        }
      }
    }

  vtkIdTypeArray* modelFaceRegionsArray = vtkIdTypeArray::New();
  modelFaceRegionsArray->SetNumberOfComponents(3);

  std::map<int, std::pair<int,int> >::iterator faceMapIter;
  for(faceMapIter = this->Internals->FaceToRegionsMap.begin();
      faceMapIter != this->Internals->FaceToRegionsMap.end(); faceMapIter++)
    {
    vtkIdType ids[3];
    ids[0] = faceMapIter->first;
    ids[1] = faceMapIter->second.first;
    ids[2] = faceMapIter->second.second;
    modelFaceRegionsArray->InsertNextTypedTuple(ids);
    }

  modelFaceRegionsArray->SetName(
    ModelParserHelper::GetModelFaceRegionsMapString());
  output->GetFieldData()->AddArray(modelFaceRegionsArray);
  modelFaceRegionsArray->Delete();

  return 1;
}


//----------------------------------------------------------------------------
void vtkCompleteShells::FindRegionEdge(vtkPolyData *input,
                                       vtkDataArray *cellNormals,
                                       vtkIntArray *regionArray,
                                       vtkIdTypeArray *modelFaceArray,
                                       vtkIdType cellIndex,
                                       char *visited)
{
  //starting from seed index, visit cell neighbors until we find an edge
  // (or run out of cells to test)

  // RSB!!!  for now, assume we have passed in valid model faces that don't
  // need to be modfied

  int currentFaceId = this->NextModelFaceId++;


  int regionId = regionArray->GetValue(cellIndex);
  std::vector<vtkIdType> cellSearchList;
  cellSearchList.push_back(cellIndex);
  visited[cellIndex] = 1;
  if (modelFaceArray->GetValue(cellIndex) == -1)
    {
    modelFaceArray->SetValue(cellIndex, currentFaceId);
    }


  int otherPtIndex;
  vtkIdType npts, *pts;
  vtkSmartPointer<vtkIdList> neighborIds = vtkSmartPointer<vtkIdList>::New();
  vtkIdType currentCellId;
  while (cellSearchList.size())
    {
    currentCellId = cellSearchList.back();
    cellSearchList.pop_back();

    input->GetCellPoints(currentCellId, npts, pts);
    if (npts < 3)
      {
      continue;
      }

    // check each edge
    for (int ptIndex = 0; ptIndex < npts; ptIndex++)
      {
      otherPtIndex = (ptIndex + 1 == npts ? 0 : ptIndex + 1);
      input->GetCellEdgeNeighbors(currentCellId, pts[ptIndex],
        pts[otherPtIndex], neighborIds);

      if (neighborIds->GetNumberOfIds() == 0)
        {
        vtkErrorMacro("Can't handle \"real\" edge!");
        return;
        }

      // if one (and only one) of the neighbors is same region as
      // the currentCell, then ignore any other edges
      int numberOfNeighborsFromSameRegion = 0;
      vtkIdType cellIndexWithMatchingRegion = 0;
      for (int j = 0; j < neighborIds->GetNumberOfIds(); j++)
        {
        if (regionArray->GetValue(neighborIds->GetId(j)) == regionId)
          {
          // RSB!!! should also be the same face... what if it isn't?
          cellIndexWithMatchingRegion = neighborIds->GetId(j);
          numberOfNeighborsFromSameRegion++;
          }
        }

      if (numberOfNeighborsFromSameRegion == 1)
        {
        if (!visited[cellIndexWithMatchingRegion])
          {
          visited[cellIndexWithMatchingRegion] = 1;
          if (modelFaceArray->GetValue(cellIndexWithMatchingRegion) == -1)
            {
            modelFaceArray->SetValue(cellIndexWithMatchingRegion, currentFaceId);
            }
          cellSearchList.push_back( cellIndexWithMatchingRegion );
          }
        continue;
        }
      else if (numberOfNeighborsFromSameRegion == 2)
        {
        vtkErrorMacro("Non-manifold with a region: NOT handled!");
        return;
        }

      // if we've gotten this far, we SHOULD (I expect to) be at the case
      // we're looking for... at least two edges/polys from another region...
      // is there a case where it could only be one?
      // here we decide which cell "to follow" as we "fix" the hole
      vtkIdType cellIdToFixHole = this->FindHoleFillingModelFace(input,
        cellNormals, currentCellId, neighborIds, pts, ptIndex, otherPtIndex);

      if (cellIdToFixHole < 0)
        {
        vtkErrorMacro("Unable to complete shell!" << currentCellId);
        continue;
        }

      int modeFaceId = modelFaceArray->GetValue(cellIdToFixHole);

      // may have already filled this hole
      int otherRegionInMap =
        this->Internals->FaceToRegionsMap[modeFaceId].second;
      if (otherRegionInMap != -1)
        {
        if (otherRegionInMap != regionId)
          {
          vtkErrorMacro("Multiple region mapping!");
          }
        }
      else
        {
        this->Internals->FaceToRegionsMap[modeFaceId].second = regionId;
        }
      }
    }
}


//----------------------------------------------------------------------------
vtkIdType vtkCompleteShells::FindHoleFillingModelFace(vtkPolyData *input,
                                                      vtkDataArray *cellNormals,
                                                      vtkIdType currentCellId,
                                                      vtkIdList *neighborIds,
                                                      vtkIdType *pts,
                                                      int ptIndex,
                                                      int otherPtIndex)
{
  double cellOrigin[3], cellNormal[3]/*, testCross[3]*/;
  cellNormals->GetTuple(currentCellId, cellNormal);
  input->GetPoints()->GetPoint(pts[0], cellOrigin);

  // assumes not concave
  double anotherTestVector[3], pt1[3], pt2[3];
  if (ptIndex == 0) // in which case otherPtIndex == 1
    {
    input->GetPoint(pts[2], pt2);
    input->GetPoint(pts[1], pt1);
    }
  else
    {
    input->GetPoint(pts[ptIndex - 1], pt2);
    input->GetPoint(pts[ptIndex], pt1);
    }
  anotherTestVector[0] = pt2[0] - pt1[0];
  anotherTestVector[1] = pt2[1] - pt1[1];
  anotherTestVector[2] = pt2[2] - pt1[2];

  vtkIdType cellIdToFixHole = -1;
  double bestDotResult = -10;
  for (int j = 0; j < neighborIds->GetNumberOfIds(); j++)
    {
    // find pt id != to our edge
    vtkIdType neighborNPts, *neighborPts;
    input->GetCellPoints(neighborIds->GetId(j), neighborNPts,
      neighborPts);
    for (int i = 0; i < neighborNPts; i++)
      {
      if (neighborPts[i] == pts[ptIndex] ||
        neighborPts[i] == pts[otherPtIndex])
        {
        continue;
        }

      // test for which side of the poly/plane we're on
      double planeEvalResult =
        vtkPlane::Evaluate(cellNormal, cellOrigin,
        input->GetPoints()->GetPoint(neighborPts[i]));

      // 99.9% of the time (probably more), this is overkill.  However, I can
      // draw case where planeEval only is insufficient.
      double neighborNormal[3];
      cellNormals->GetTuple(neighborIds->GetId(j), neighborNormal);
      double anotherTest = vtkMath::Dot(anotherTestVector, neighborNormal);

      if ((planeEvalResult < 0 && anotherTest < 0) ||
        (planeEvalResult > 0 && anotherTest > 0))
        {
        // not a candidate
        break;
        }

      if (planeEvalResult < 0 || bestDotResult < -1 || anotherTest == 0)
        {
        double dotResult = vtkMath::Dot(cellNormal, neighborNormal);

        // if anotherTest is zero, then normal must be opposite...
        // planeEvalResult is probably also 0 or close to it.  Is this
        // actually going to happen?  anyway, set dotResult so that it
        // falls in between planeEvanResults from below and above the plane
        if (anotherTest == 0)
          {
          dotResult = -1.5;
          }
        // dotResult will be between -1 and 1... and if we're on the
        // side of the plan we expect (will usually) be on as far at the
        // cell we're interested in finding... the "inside" or
        // planeEvalResult < 0... we can directly use the calue returned.
        // If on the positive side, which COULD (but unlikely) happen,
        // we need to shift the result such that -1 is larger than 1, but
        // also that it is less than -1 (becuase we still expect to find
        // a cell on the negative side of the plane to use).  Could be
        // "-2 - dotResult", but went with -3 just to give a bit of a gap
        // betwee nresult on the negative and positive side of the plane
        else if (planeEvalResult > 0)
          {
          dotResult = -3 - dotResult;
          }

        if (dotResult > bestDotResult)
          {
          cellIdToFixHole = neighborIds->GetId(j);
          }
        }
      // this seems odd, but we only need to test one point from this cell
      break;
      }
    }

  return cellIdToFixHole;
}

//-----------------------------------------------------------------------------
void vtkCompleteShells::FindClosestEnclosingRegion(int regionId,
                                                   vtkIdType modelFaceId,
                                                   vtkPolyData *input,
                                                   vtkDataArray *cellNormals,
                                                   vtkIntArray* regionArray,
                                                   vtkIdTypeArray *modelFaceArray,
                                                   vtkCellLocator *locator)
{
  // 1st, construct polydata of the region, to give us position(s) to
  // look outwards from...
  vtkIdType numCells = input->GetNumberOfCells();

  vtkSmartPointer<vtkPolyData> tmpPD = vtkSmartPointer<vtkPolyData>::New();
  tmpPD->SetPoints( input->GetPoints() );
  vtkCellArray *polys = vtkCellArray::New();
  polys->Allocate( 4 * numCells );  // should be more than enough, as it amount to the whole
  tmpPD->SetPolys( polys );
  polys->FastDelete();

  vtkIdType cellDataIndex = input->GetNumberOfVerts() + input->GetNumberOfLines();
  vtkCellArray *inputPolys = input->GetPolys();
  vtkIdType npts, *pts;
  for (inputPolys->InitTraversal(); inputPolys->GetNextCell(npts, pts); cellDataIndex++)
    {
    if ( regionArray->GetValue( cellDataIndex ) == regionId )
      {
      polys->InsertNextCell(npts, pts);
      }
    }

  // use a cell locator to find  a "good" cell to search from (when looking
  // for intersecting cells along a line, we won't hit this region; well, will
  // at t = 0, but can then ignore it)
  vtkSmartPointer<vtkCellLocator> localLocator = vtkSmartPointer<vtkCellLocator>::New();
  localLocator->SetDataSet( tmpPD );
  localLocator->BuildLocator();

  // come up with 6 possible search points (-x, +x, -y, +y, -z, +z)
  double *bounds = tmpPD->GetBounds();
  double midX = (bounds[0] + bounds[1]) / 2.0;
  double midY = (bounds[2] + bounds[3]) / 2.0;
  double midZ = (bounds[4] + bounds[5]) / 2.0;
  double locatorSeedPt[6][3] = { {bounds[0], midY, midZ}, {bounds[1], midY, midZ},
                                 {midX, bounds[2], midZ}, {midX, bounds[3], midZ},
                                 {midX, midY, bounds[4]}, {midX, midY, bounds[5]} };
  double *inputBounds = input->GetBounds();
  double searchDirection[6][3] = { {inputBounds[0] - inputBounds[1], 0, 0},
                                   {inputBounds[1] - inputBounds[0], 0, 0},
                                   {0, inputBounds[2] - inputBounds[3], 0},
                                   {0, inputBounds[3] - inputBounds[2], 0},
                                   {0, 0, inputBounds[4] - inputBounds[5]},
                                   {0, 0, inputBounds[5] - inputBounds[4]} };
  // test all 6 directions... -1 if outside (or inconclusive);
  // otherwise, regionId of enclosing region; added to resultMap at loop end
  int result;
  std::map<int, int> resultMap;
  std::map<int, int>::iterator resultIter;

  double closestPt[3], dist2, endPt[3];
  vtkIdType cellId;
  int subId;
  vtkSmartPointer<vtkIdList> candidateCells = vtkSmartPointer<vtkIdList>::New();
  for (int i = 0; i < 6; i++)
    {
    result = -1; // default to not in another
    localLocator->FindClosestPoint( locatorSeedPt[i], closestPt, cellId, subId, dist2 );

    // search along line segment starting at "closestPt", to the boundary of
    // the whole dataset along the searchDirection.
    endPt[0] = closestPt[0] + searchDirection[i][0];
    endPt[1] = closestPt[1] + searchDirection[i][1];
    endPt[2] = closestPt[2] + searchDirection[i][2];
    locator->FindCellsAlongLine(locatorSeedPt[i], endPt, 0.0, candidateCells);

    // Now, reduce list of cells to only those that intersect the line, and
    // sort them based on their t value; not sure a multimap is actually
    // necessary, but not SURE that we won't get duplicate t values..
    std::multimap<double, vtkIdType> intersectingCellMap;
    double t, x[3], pcoords[3];
    for (vtkIdType j = 0; j < candidateCells->GetNumberOfIds(); j++)
      {
      if ( input->GetCell( candidateCells->GetId(j) )->IntersectWithLine(
        locatorSeedPt[i], endPt, 0.0, t, x, pcoords, subId) )
        {
        intersectingCellMap.insert(
          std::pair< double, vtkIdType >(t, candidateCells->GetId(j)) );
        }
      }

    // not handling intesection on edge... but unlikely!

    std::multimap<double, vtkIdType>::iterator cellIter;
    for (cellIter = intersectingCellMap.begin();
      cellIter != intersectingCellMap.end(); cellIter++)
      {
      std::map<int, std::pair<int, int> >::const_iterator faceRegionMapIter =
        this->Internals->FaceToRegionsMap.find( modelFaceArray->GetValue( cellIter->second ) );

      // the 1st region associated with the face (and thus normal point out for it)
      int testCellRegionId = faceRegionMapIter->second.first;
      if (testCellRegionId == regionId)
        {
        // intersection with start of line on the region we're testing;
        // note, also expect t == 0, but not testing it;  skip/continue...
        continue;
        }

      // so, test direction of our line (from start to end), versus the normal
      // of the cell we intersected:
      //  - if only one region uses the face, AND
      //     - the dot product is greater than zero, inside the region
      //     - dot product <= 0, either inconclusive or outside the region;
      //       in this case, we skip all other cells until we leave the region
      //       again
      //  - if multiple regions use the face,
      //     - dot > 0, use the 1st region
      //     - dot <= 0, second region
      double cellNormal[3];
      cellNormals->GetTuple(cellIter->second, cellNormal);
      if (vtkMath::Dot( cellNormal, searchDirection[i] ) > 0)
        {
        result = testCellRegionId;
        break;
        }
      else if ( faceRegionMapIter->second.second != -1 )
        {
        result = faceRegionMapIter->second.second;
        break;
        }
      else  // dot <= 0 AND only one region uses the face (so far)
        {
        // we're "entering" another shell/region that does not enclose the
        // region we're testing... skip all intersecting cells until we "leave"
        // the region.
        while ( ++cellIter != intersectingCellMap.end() )
          {
          if ( regionArray->GetValue( cellIter->second ) == testCellRegionId )
            {
            // does normal indicate we're "leaving"; if so, break
            // (and continue with the "main" loop)
            cellNormals->GetTuple(cellIter->second, cellNormal);
            if (vtkMath::Dot( cellNormal, searchDirection[i] ) > 0)
              {
              break;
              }
            }
          }
        }
      }

    resultIter = resultMap.find(result);
    if ( resultIter == resultMap.end() )
      {
      resultMap[ result ] = 1;
      }
    else
      {
      resultIter->second++;
      }
    }

  // use result that appears most, but hope for unanimous; potentially
  // warn user if not unanimous (depending on value of
  // MinimumSubmergedVoteCountToAvoidWarning)
  int resultRegion = -1, maxCount = 0;
  for (resultIter = resultMap.begin(); resultIter != resultMap.end(); resultIter++)
    {
    if ( resultIter->second > maxCount )
      {
      resultRegion = resultIter->first;
      maxCount = resultIter->second;
      }
    }

  if (maxCount < this->MinimumSubmergedVoteCountToAvoidWarning)
    {
    vtkWarningMacro("Unaniomous decision (of 6) on interior test of solid not obtained (region, count): (" <<
      regionId << ", " << maxCount << ")");
    }

  // save the result of our search
  this->Internals->FaceToRegionsMap[ modelFaceId ].second = resultRegion;
}

//----------------------------------------------------------------------------
void vtkCompleteShells::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ModelRegionArrayName: " << this->ModelRegionArrayName << endl;
  os << indent << "ModelFaceArrayName: " << this->ModelFaceArrayName << endl;
  os << indent << "DetectAndFixSubmergedSolids: " << (this->DetectAndFixSubmergedSolids ? "On" : "Off") << endl;
  os << indent << "MinimumSubmergedVoteCountToAvoidWarning: " << this->MinimumSubmergedVoteCountToAvoidWarning << endl;
}
