//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkSplitPlanarLines.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkIncrementalOctreePointLocator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkSortDataArray.h"
#include "vtkVector.h"
#include "vtkVectorOperators.h"

#include "smtk/extension/vtk/meshing/vtkRayIntersectionLocator.h"
#include <map>
#include <vector>

struct SegmentRecord
{
  vtkIdType CellId;
  int SubId;
  double Param[2];
  vtkVector3d Point;
};

typedef std::vector<SegmentRecord> SegmentRecords;
typedef std::map<vtkIdType,SegmentRecords> HitList;

vtkStandardNewMacro(vtkSplitPlanarLines);

vtkSplitPlanarLines::vtkSplitPlanarLines()
{
  this->Tolerance = 0.;
}

vtkSplitPlanarLines::~vtkSplitPlanarLines()
{
}

void vtkSplitPlanarLines::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Tolerance: " << this->Tolerance << "\n";
}


inline void EraseHit(
  std::vector<vtkVector3d>& points,
  std::vector<vtkVector3d>& pcoords,
  std::vector<double>& tvals,
  std::vector<vtkIdType>& cellIds,
  std::vector<int>& subIds,
  vtkIdType i,
  vtkIdType& nhits)
{
  tvals.erase(tvals.begin() + i);
  points.erase(points.begin() + i);
  pcoords.erase(pcoords.begin() + i);
  cellIds.erase(cellIds.begin() + i);
  subIds.erase(subIds.begin() + i);
  --nhits;
}

void IntersectSegments(
  vtkPolyData* input,
  vtkIdType cellId,
  vtkRayIntersectionLocator* clocator,
  HitList& hits,
  vtkPolyData* output,
  vtkIncrementalOctreePointLocator* plocator,
  vtkIdTypeArray* pedigreeIds)
{
  vtkIdType npts;
  vtkIdType* conn;
  input->GetCellPoints(cellId, npts, conn);
  if (npts < 2)
    {
    return;
    }
  vtkPoints* opts = output->GetPoints();
  std::vector<vtkVector3d> points;
  std::vector<vtkVector3d> pcoords;
  std::vector<double> tvals;
  std::vector<vtkIdType> cellIds;
  std::vector<int> subIds;
  vtkVector3d ptA;
  vtkVector3d ptB;
  vtkIdType nhits;
  vtkCellData* icd = input->GetCellData();
  vtkCellData* cd = output->GetCellData();
  vtkPointData* pd = output->GetPointData();
  opts->GetPoint(conn[0], ptA.GetData());
  for (int j = 1; j < npts; ++j)
    {
    opts->GetPoint(conn[j], ptB.GetData());
    clocator->AllIntersectionsAlongSegment(ptA, ptB, points, tvals, pcoords, cellIds, subIds);
    // Now we must remove hits from cells with an ID less than cellId
    // (those are kept in hits and should be commutative).
    // Hits at exactly t=0 or t=1 must be discarded to avoid
    // degenerate output segments.
    nhits = static_cast<vtkIdType>(cellIds.size());
    HitList::iterator hitit;
    for (vtkIdType i = 0; i < nhits; )
      {
      bool notErased = true;
      if (
        cellIds[i] < cellId ||
        (cellIds[i] == cellId && subIds[i] < j - 1))
        { // Remove the hit. If it really exists, it is already stored in "hits".
        EraseHit(points, pcoords, tvals, cellIds, subIds, i, nhits);
        notErased = false;
        }
      else
        {
        // We are responsible for this hit...
        // add it to other cell's entries in hits.
        if (pcoords[i][0] > 0. && pcoords[i][0] < 1.)
          {
          hitit = hits.find(cellIds[i]);
          if (hitit == hits.end())
            {
            SegmentRecords empty;
            hitit = hits.insert(
              std::pair<vtkIdType,SegmentRecords>(cellIds[i], empty)).first;
            }
          SegmentRecord srec;
          srec.CellId = cellIds[i];
          srec.SubId = subIds[i];
          srec.Param[0] = pcoords[i][0];
          srec.Param[1] = tvals[i];
          srec.Point = points[i];
          hitit->second.push_back(srec);
          }
        }
      // We might have recorded the hit for another cell above,
      // but if it's at the beginning or end of *this* segment,
      // we don't need it:
      if ((tvals[i] == 0. || tvals[i] == 1.) && notErased)
        {
        EraseHit(points, pcoords, tvals, cellIds, subIds, i, nhits);
        notErased = false;
        }
      if (notErased)
        {
        ++i;
        }
      }
    // We must also add in relevant entries from the "hits" map,
    // for which the other cell was responsible.
    // Having one cell responsible for each possible pair of
    // intersections eliminates problems where an intersection is
    // detected when testing one segment and not when testing the
    // other.
    hitit = hits.find(cellId);
    if (hitit != hits.end())
      {
      SegmentRecords::iterator sit;
      for (sit = hitit->second.begin(); sit != hitit->second.end(); ++sit)
        {
        if (sit->SubId == j - 1)
          {
          points.push_back(sit->Point);
          tvals.push_back(sit->Param[0]);
          subIds.push_back(sit->SubId);
          cellIds.push_back(sit->CellId);
          pcoords.push_back(vtkVector3d(sit->Param[1], 0., 0.));
          SegmentRecords::iterator tmp = sit - 1;
          hitit->second.erase(sit);
          sit = tmp;
          }
        }
      }
    // Finally, we must sort all the lists by tvals in order to
    // report subsegments correctly.
    vtkNew<vtkIdList> permutation;
    nhits = static_cast<vtkIdType>(cellIds.size());
    permutation->SetNumberOfIds(nhits);
    for (vtkIdType i = 0; i < nhits; ++i)
      {
      permutation->SetId(i, i);
      }
    vtkNew<vtkDoubleArray> twrapper;
    twrapper->SetArray(&tvals[0], nhits, 1);
    vtkSortDataArray::Sort(twrapper.GetPointer(), permutation.GetPointer());
    // Output the sorted segments.
    vtkIdType seg[2];
    seg[0] = plocator->FindClosestPoint(ptA.GetData());
    for (vtkIdType i = 0; i < nhits; ++i)
      {
      if ( i > 0 && tvals[i] == tvals[i - 1])
        {
        continue; // Ignore duplicate t-values. Yes, this happens.
        }
      if (plocator->InsertUniquePoint((ptA + tvals[i]*(ptB - ptA)).GetData(), seg[1]))
        { // Interpolate along edge to get new point data
        pd->InterpolateEdge(pd, seg[1], conn[j - 1], conn[j], tvals[i]);
        }
      if (seg[0] != seg[1])
        {
        vtkIdType segId = output->GetLines()->InsertNextCell(2, seg);
        cd->CopyData(icd, cellId, segId);
        if (pedigreeIds)
          {
          pedigreeIds->InsertNextValue(cellId);
          }
        seg[0] = seg[1];
        }
      }
    if (nhits == 0 || tvals[nhits - 1] < 1.)
      {
      seg[1] = plocator->FindClosestPoint(ptB.GetData());
      if (seg[0] != seg[1])
        {
        vtkIdType segId = output->GetLines()->InsertNextCell(2, seg);
        if (pedigreeIds)
          {
          pedigreeIds->InsertNextValue(cellId);
          }
        cd->CopyData(icd, cellId, segId);
        }
      }

    // Clear lists for next segment
    points.clear();
    tvals.clear();
    pcoords.clear();
    cellIds.clear();
    subIds.clear();
    // Reuse ptA as the endpoint of the next segment in the polyline.
    ptA = ptB;
    }
}

int vtkSplitPlanarLines::RequestData(
  vtkInformation* /*req*/,
  vtkInformationVector** inVec,
  vtkInformationVector* outVec)
{
  // Get input and output data for request:
  vtkInformation* inInfo = inVec[0]->GetInformationObject(0);
  vtkInformation* outInfo = outVec->GetInformationObject(0);
  vtkPolyData* input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData* output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // Build locator on input so GetCellPoints() works.
  input->BuildCells();

  vtkNew<vtkRayIntersectionLocator> clocator;
  clocator->SetDataSet(input);
  clocator->SetTolerance(this->Tolerance);
  clocator->BuildLocator();

  vtkIdType numVerts = input->GetNumberOfVerts();
  vtkIdType numLines = input->GetNumberOfLines();
  vtkCellArray* lines = input->GetLines();
  vtkNew<vtkPoints> opts;
  vtkNew<vtkCellArray> olines;
  // Copy input points and attributes since we'll be adding intersection points.
  opts->DeepCopy(input->GetPoints());
  output->SetPoints(opts.GetPointer());
  output->SetLines(olines.GetPointer());
  output->GetPointData()->DeepCopy(input->GetPointData());
  output->GetCellData()->CopyAllocate(input->GetCellData());
  // Initialize event queue with polyline endpoints
  lines->InitTraversal();
  HitList hits; // intersections we've already processed
  vtkIdType firstPolyCell = numVerts + numLines;
  vtkNew<vtkIncrementalOctreePointLocator> plocator;
  vtkNew<vtkIdTypeArray> pedigreeIds;
  plocator->SetDataSet(output);
  plocator->SetTolerance(this->Tolerance);
  pedigreeIds->SetName("vtkPedigreeIds");
  bool haveInputPedigree = input->GetCellData()->GetPedigreeIds() ? true : false;
  for (vtkIdType cellId = numVerts; cellId < firstPolyCell; ++cellId)
    {
    IntersectSegments(
      input, cellId, clocator.GetPointer(), hits, output, plocator.GetPointer(),
      haveInputPedigree ? NULL : pedigreeIds.GetPointer());
    }
  // Only add cell pedigree Ids if the input had none.
  // If the input had them, then copying cell data to each
  // output segment will leave the existing ones intact.
  if (!haveInputPedigree)
    {
    output->GetCellData()->SetPedigreeIds(pedigreeIds.GetPointer());
    }

  return 1;
}
