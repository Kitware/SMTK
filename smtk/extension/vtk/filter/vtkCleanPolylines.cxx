//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/filter/vtkCleanPolylines.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCleanPolyData.h"
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataWriter.h"
#include "vtkStripper.h"

#include <map>
#include <set>

vtkStandardNewMacro(vtkCleanPolylines);

vtkCleanPolylines::vtkCleanPolylines()
{
  this->MinimumLineLength = 5;
  this->UseRelativeLineLength = true;
}

int vtkCleanPolylines::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // get rid of any duplicated points
  vtkNew<vtkCleanPolyData> cleaner;
  cleaner->SetInputData( input );
  cleaner->Update();

  // Now strip the lines an remove any self intersections
  vtkNew<vtkPolyData> initialLines;
  vtkNew<vtkPolyData> strippedLines;
  // We only want the lines
  initialLines->SetPoints(cleaner->GetOutput()->GetPoints());
  initialLines->SetLines(cleaner->GetOutput()->GetLines());
  initialLines->BuildLinks();

  vtkNew<vtkDoubleArray> lengths;
  lengths->SetNumberOfComponents(1);
  // Create polylines that do not self intersect
  this->StripLines(initialLines.GetPointer(), strippedLines.GetPointer(),
                   lengths.GetPointer());
  strippedLines->BuildLinks();

  // Remove non-manifold features
  vtkNew<vtkPolyData> reducedPolylines;
  vtkNew<vtkDoubleArray> lengths1;
  lengths->SetNumberOfComponents(1);
  this->RemoveNonManifoldFeatures(strippedLines.GetPointer(), lengths.GetPointer(),
                                  reducedPolylines.GetPointer(),
                                  lengths1.GetPointer());

  double minimumPolylineLength = this->MinimumLineLength;
  double l;
  vtkIdType npts, *ptIds, i, n = lengths1->GetNumberOfTuples();
  if (this->UseRelativeLineLength)
    {
    double sum = 0;
    double nCells = static_cast<double>(input->GetNumberOfLines());
    for (i = 0; i < n; i++)
      {
      sum += lengths1->GetValue(i);
      }

    double averageLength = sum / nCells;
    minimumPolylineLength *= averageLength;
    }

  std::multimap<double, vtkIdType> lineMap; // for easy sorting of lines by length
  vtkCellArray *lines = reducedPolylines->GetLines();
  lines->InitTraversal();
  vtkIdType traverseLocation = 0, memoryRequirement = 0;
  i = 0;
  while (lines->GetNextCell(npts, ptIds))
    {
    l = lengths1->GetValue(i);
    if (l > minimumPolylineLength)
      {
      lineMap.insert(std::pair<double, vtkIdType>(l, traverseLocation));
      memoryRequirement += npts + 1;
      }
    traverseLocation = lines->GetTraversalLocation();
    i++;
    }

  vtkNew<vtkPolyData> tmpPD;
  tmpPD->SetPoints( strippedLines->GetPoints() );
  vtkNew<vtkCellArray> outputLines;
  outputLines->Allocate( memoryRequirement );
  tmpPD->SetLines( outputLines.GetPointer() );
  vtkNew<vtkDoubleArray> lineLengthArray;
  lineLengthArray->SetName("LineLength");
  lineLengthArray->SetNumberOfComponents(1);
  tmpPD->GetCellData()->AddArray(lineLengthArray.GetPointer());

  vtkNew<vtkIntArray> contourIndices;
  contourIndices->SetNumberOfComponents( 1 );
  contourIndices->SetName("LineIndex");
  tmpPD->GetCellData()->SetScalars( contourIndices.GetPointer() );

  std::multimap<double, vtkIdType>::reverse_iterator mapIter;
  for (mapIter = lineMap.rbegin(); mapIter != lineMap.rend(); mapIter++)
    {
    lines->GetCell(mapIter->second, npts, ptIds);
    vtkIdType contourIndex = outputLines->InsertNextCell(npts, ptIds);
    lineLengthArray->InsertNextValue(mapIter->first);
    contourIndices->InsertNextValue( contourIndex );
    }

  // get rid of unused points
  vtkNew<vtkCleanPolyData> removeUnusedPoints;
  removeUnusedPoints->PointMergingOff(); // probably not necessary
  removeUnusedPoints->SetInputData( tmpPD.GetPointer() );
  removeUnusedPoints->Update();
  output->ShallowCopy( removeUnusedPoints->GetOutput() );
  return 1;
}

void vtkCleanPolylines::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Minimum Line Length: " << this->MinimumLineLength << "\n";
  os << indent << "Use Relative Line Length: " << (this->UseRelativeLineLength ? "On" : "Off") << endl;
}

void vtkCleanPolylines::StripLines(vtkPolyData *input, vtkPolyData *result,
                                   vtkDoubleArray *lengths)
{
  //Create a visited mask for all point ids - we need this in the case
  // there are any closed loops that are not connected to any other line
  vtkPoints *pnts = input->GetPoints();
  if(pnts == NULL)
  {
    return;
  }
  vtkIdType numPts = pnts->GetNumberOfPoints();
  unsigned char *marks = new unsigned char[numPts];
  // Mark all points as not being visited
  memset(marks, 0, numPts);

  // Prep the result
  result->SetPoints(pnts);
  vtkNew<vtkIdList> ids;
  vtkNew<vtkCellArray> plines;
  result->SetLines(plines.GetPointer());
  // We need a map for non-manifold Points
  std::set<vtkIdType> vCells; // A set of visited cells
  std::pair<std::set<vtkIdType>::iterator, bool> ret;
  unsigned short nCells;
  // Scan each point in the input for a cell count not equal to 2 (non-manifold point)
  vtkIdType pid, lastLineId, *pCells;
  unsigned short i;
  double l;
  for (pid = 0; pid < numPts; pid++)
    {
    input->GetPointCells(pid, nCells, pCells);
    if ((!nCells) || (nCells == 2))
      {
      continue; // Manifold Point
      }
    for (i = 0; i < nCells; i++)
      {
      // Have we walked along this cell?
      ret = vCells.insert(pCells[i]);
      if (!ret.second)
        {
        // We already had visited this cell so we can skip it
        continue;
        }
      this->TraverseLine(pid, pCells[i], input, marks, ids.GetPointer(), &l, &lastLineId);
      // insert the cell and the length
      plines->InsertNextCell(ids.GetPointer());
      lengths->InsertNextTuple1(l);
      // Mark the last cell in the polyline as being visited
      vCells.insert(lastLineId);
      }
    }
  // Now we need to go through the points one more time to look for loops
  for (pid = 0; pid < numPts; pid++)
    {
    // Have we already visited the point?
    if (marks[pid])
      {
      continue;
      }
    input->GetPointCells(pid, nCells, pCells);
    if (nCells != 2)
      {
      continue; // Non-Manifold Point
      }
    this->TraverseLine(pid, pCells[0], input, marks, ids.GetPointer(), &l, &lastLineId);
    // insert the cell and the length
    plines->InsertNextCell(ids.GetPointer());
    lengths->InsertNextTuple1(l);
    }
  delete[] marks;
}

void vtkCleanPolylines::TraverseLine(vtkIdType startPid, vtkIdType startCellId,
                                     vtkPolyData *input, unsigned char *marks,
                                     vtkIdList *ids, double *length,
                                     vtkIdType *lastLineId)
{
  vtkIdType pid, lastPid, cell, *pntCells;
  unsigned short nCells;
  vtkIdType nPnts, *cellPnts;
  double p0[3], p1[3];
  *length = 0.0;
  ids->Reset();
  ids->InsertNextId(startPid);
  marks[startPid] = 1;
  lastPid = startPid;
  cell = startCellId;
  input->GetPoint(startPid, p0);

  for (;;)
    {
    input->GetCellPoints(cell, nPnts, cellPnts);
    // See which is the next point in the polyline
    if (cellPnts[0] == lastPid)
      {
      pid = cellPnts[1];
      }
    else
      {
      pid = cellPnts[0];
      }

    input->GetPoint(pid, p1);
    *length += sqrt(vtkMath::Distance2BetweenPoints(p0, p1));

    // add point to polyline
    ids->InsertNextId(pid);
    marks[pid] = 1;
    *lastLineId = cell;

    // Have we looped back to the start?
    if (pid == startPid)
      {
      return;
      }
    input->GetPointCells(pid, nCells, pntCells);
    // Have we come to a non-manifold junction?
    if (nCells != 2)
      {
      return;
      }
    // Get the next cell
    if (pntCells[0] == cell)
      {
      cell = pntCells[1];
      }
    else
      {
      cell = pntCells[0];
      }
    lastPid = pid;
    p0[0] = p1[0];
    p0[1] = p1[1];
    p0[2] = p1[2];
    }
}

void vtkCleanPolylines::RemoveNonManifoldFeatures(vtkPolyData *input, vtkDoubleArray *lengths,
                                                  vtkPolyData *result,
                                                  vtkDoubleArray *newLengths)
{
  //Create a visited mask for all point ids - we need this in the case
  // there are any closed loops that are not connected to any other line
  vtkPoints *pnts = input->GetPoints();
  vtkCellArray *cells = input->GetLines();
  vtkIdType numCells = cells->GetNumberOfCells();
  unsigned char *marks = new  unsigned char[numCells];
  // Mark all cells as being kept
  memset(marks, 1, numCells);

  // Prep the result
  result->SetPoints(pnts);
  vtkNew<vtkIdList> ids;
  vtkNew<vtkCellArray> plines;
  result->SetLines(plines.GetPointer());
  unsigned short nCells;
  // Scan each point in the input for a cell count not equal to 2 (non-manifold point)
  vtkIdType *pCells, cell, nPts, *cPnts;
  unsigned short i, j;
  double l;
  double cellLengths[2];
  vtkIdType cellIds[2], testPoints[2];
  // In this first pass we are moving completed loops to the
  // result and marking cells as rejected if they are not
  // the 2 longest lines coming into a point
  for (cell = 0; cell < numCells; cell++)
    {
    // Has this cell been marked as rejected?
    if (!marks[cell])
      {
      continue;
      }

    input->GetCellPoints(cell, nPts, cPnts);
    testPoints[0] = cPnts[0];
    testPoints[1] = cPnts[nPts-1];
    // Are we dealing with a simple loop?
    if (testPoints[0] == testPoints[1])
      {
      plines->InsertNextCell(nPts, cPnts);
      newLengths->InsertNextTuple1(lengths->GetValue(cell));
      marks[cell] = 2;  // Marked as moved
      continue;
      }
    // We are dealing with non-manifold vertices
    for (j = 0; j < 2; j++)
      {
      // check to see if the point is "non-manifold"
      input->GetPointCells(testPoints[j], nCells, pCells);
      // Find the 2 longest cells
      cellLengths[0] = cellLengths[1] = -1.0;
      cellIds[0] = cellIds[1] = -1;
      for (i = 0; i < nCells; i++)
        {
        if (!marks[pCells[i]])
          {
          // skip rejected cells
          continue;
          }
        l = lengths->GetValue(pCells[i]);
        if (l > cellLengths[0])
          {
          // Reject cellIds[1]
          if (cellIds[1] != -1)
            {
            marks[cellIds[1]] = 0; // rejected
            }
          cellLengths[1] = cellLengths[0];
          cellIds[1] = cellIds[0];
          cellLengths[0] = l;
          cellIds[0] = pCells[i];
          }
        else if (l > cellLengths[1])
          {
          // Reject cellIds[1]
          if (cellIds[1] != -1)
            {
            marks[cellIds[1]] = 0; // rejected
            }
          cellLengths[1] = l;
          cellIds[1] = pCells[i];
          }
        else
          {
          // reject this cell
          marks[pCells[i]] = 0;
          }
        }
      }
    }

  // Now we go through the cells one more time - in this case
  // we append polylines that meet at a point
  vtkIdType cellCounts[2];
  for (cell = 0; cell < numCells; cell++)
    {
    // Has this cell been marked as rejected?
    if (!marks[cell])
      {
      continue;
      }

    // Has this cell been moved already?
    if (marks[cell] == 2)
      {
      continue;
      }

    // Get the number of non-rejected cells coming into both end points
    input->GetCellPoints(cell, nPts, cPnts);
    cellCounts[0] = cellCounts[1] = 0;
    testPoints[0] = cPnts[0];
    testPoints[1] = cPnts[nPts-1];

    for (j = 0; j < 2; j++)
      {
      // check to see if the point is "non-manifold"
      input->GetPointCells(testPoints[j], nCells, pCells);
      for (i = 0; i < nCells; i++)
        {
        if (marks[pCells[i]] == 1)
          {
          ++cellCounts[j];
          }
        }
      }
    if (cellCounts[0] == 1 && cellCounts[1] == 1)
      {
      // This is an isolated loop - just move it
      plines->InsertNextCell(nPts, cPnts);
      newLengths->InsertNextTuple1(lengths->GetValue(cell));
      marks[cell] = 2;  // Marked as moved
      continue;
      }

    if (cellCounts[0] == 2 && cellCounts[1] == 2)
      {
      // This cell is connected to 2 other cells and will get traversed later
      continue;
      }
    // OK - this cell is at the end of a set of cells so lets walk it
    if (cellCounts[0] == 1)
      {
      this->TraversePolyLine(testPoints[0], cell, input, lengths,
                             marks, ids.GetPointer(), &l);
      plines->InsertNextCell(ids.GetPointer());
      newLengths->InsertNextTuple1(l);
      }
    }
  delete[] marks;
}

void vtkCleanPolylines::TraversePolyLine(vtkIdType startPid, vtkIdType startCellId,
                                         vtkPolyData *input, vtkDoubleArray *lengths,
                                         unsigned char *marks,
                                         vtkIdList *ids, double *length)
{
  vtkIdType pid, lastPid, cell, *pntCells;
  unsigned short nCells;
  int i;
  vtkIdType nPnts, *cellPnts;
  *length = 0.0;
  ids->Reset();
  ids->InsertNextId(startPid);
  lastPid = startPid;
  cell = startCellId;

  for (;;)
    {
    // Add the length of the cell
    *length += lengths->GetValue(cell);
    input->GetCellPoints(cell, nPnts, cellPnts);
    // See which is the next point in the polyline and add its points
    // to the ids
    if (cellPnts[0] == lastPid)
      {
      // OK we are traversing the polyline from first to last
      for (i = 1; i < nPnts; i++)
        {
        ids->InsertNextId(cellPnts[i]);
        }
      pid = cellPnts[nPnts-1];
      }
    else // Traversing the cell backwards
      {
      for (i = nPnts-1; i > -1; i--)
        {
        ids->InsertNextId(cellPnts[i]);
        }
      pid = cellPnts[0];
      }
    // Mark the cell has having been moved
    marks[cell] = 2;

    // Have we looped back to the start?
    if (pid == startPid)
      {
      return;
      }
    input->GetPointCells(pid, nCells, pntCells);
    // Find the next cell that has not been rejected or moved
    cell = -1;
    for (i = 0; i < nCells; i++)
      {
      if (marks[pntCells[i]] == 1)
        {
        cell = pntCells[i];
        break;
        }
      }
    // Have we reached the end?
    if (cell == -1)
      {
      return;
      }
    lastPid = pid;
    }
}
