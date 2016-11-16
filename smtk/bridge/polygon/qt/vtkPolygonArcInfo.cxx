//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/polygon/qt/vtkPolygonArcInfo.h"

#include "vtkAlgorithmOutput.h"
#include "vtkCellArray.h"
#include "vtkClientServerStream.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDataObject.h"
#include "vtkDataObjectTreeIterator.h"
#include "vtkObjectFactory.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPolyData.h"
#include "vtkInformation.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

vtkStandardNewMacro(vtkPolygonArcInfo);

//----------------------------------------------------------------------------
vtkPolygonArcInfo::vtkPolygonArcInfo()
{
  this->BlockIndex = -1;
  this->ClosedLoop = 0;
  this->NumberOfPoints = 0;
  this->ModelEntityID = NULL;
  this->SelectedPointId = -1;
  this->SelectedPointCoordinates[0] =
  this->SelectedPointCoordinates[1] = 
  this->SelectedPointCoordinates[2] = 0.0;
}

//----------------------------------------------------------------------------
vtkPolygonArcInfo::~vtkPolygonArcInfo()
{
  this->SetModelEntityID(NULL);
}

//----------------------------------------------------------------------------
void vtkPolygonArcInfo::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "NumberOfPoints: " << this->NumberOfPoints << endl;
  os << indent << "ClosedLoop: " << this->ClosedLoop << endl;
  os << indent << "BlockIndex: " << this->BlockIndex << endl;
  os << indent << "ModelEntityID: " << this->ModelEntityID << endl;
  os << indent << "SelectedPointId: " << this->SelectedPointId << endl;
  os << indent << "SelectedPointCoordinates: "
               << this->SelectedPointCoordinates[0] << ", "
               << this->SelectedPointCoordinates[1] << ", "
               << this->SelectedPointCoordinates[2] << endl;
}

//----------------------------------------------------------------------------
void vtkPolygonArcInfo::CopyFromObject(vtkObject* obj)
{
  //reset member variables to defaults
  this->ClosedLoop = false;
  this->NumberOfPoints = 0;
  this->SetModelEntityID(NULL);
  vtkMultiBlockDataSetAlgorithm* filterAlg =
    vtkMultiBlockDataSetAlgorithm::SafeDownCast(obj);
  if(!filterAlg)
    return;
/*
  vtkModelMultiBlockSource *modelsource = vtkModelMultiBlockSource::SafeDownCast(
    filterAlg->GetInputConnection(0, 0)->GetProducer());
  if (!modelsource)
    {
    return;
    }
  this->SetModelEntityID(modelsource->GetModelEntityID());
*/
  vtkMultiBlockDataSet *mbds = filterAlg->GetOutput();
  if (!mbds || this->BlockIndex < 0)
    {
    return;
    }
  vtkCompositeDataIterator* iter = mbds->NewIterator();
  iter->SetSkipEmptyNodes(false);
  vtkDataObjectTreeIterator* treeIter = vtkDataObjectTreeIterator::SafeDownCast(iter);
  if(treeIter)
  {
    treeIter->VisitOnlyLeavesOff();
  }
  int index = 0;
  vtkPolyData* edgePoly = NULL;
  vtkInformation* metaInfo = NULL;
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem(), index++)
    {
    if (index == this->BlockIndex)
      {
      edgePoly = vtkPolyData::SafeDownCast( iter->GetCurrentDataObject());
      if(iter->HasCurrentMetaData())
        metaInfo = iter->GetCurrentMetaData();
      break;
      }
    }
  iter->Delete();

  if(!edgePoly || edgePoly->GetNumberOfPoints() <= 1
     || !edgePoly->GetNumberOfCells()
     || !edgePoly->GetLines())
    {
    vtkErrorMacro("The selected edge does not have valid geometry!");
    return;
    }
  smtk::common::UUID edgeId = vtkModelMultiBlockSource::GetDataObjectUUID(metaInfo);
  if(!edgeId.isNull())
    {
    this->SetModelEntityID(edgeId.toString().c_str());
    }
  else
    {
    vtkErrorMacro("Invalid edge UUID in block meta data information object!");
    return;
    }

  // figure out whether the polyline is a closed loop.
  // We are assuming the cell points are in a proper order for this to work.
  // Check the first point in the first cell with the last point in the last cell
  vtkCellArray* lines = edgePoly->GetLines();
  vtkIdType numCells = lines->GetNumberOfCells();
  if(numCells == 0)
    return;
  double p1[3], p2[3];
  vtkIdType *pts,npts;
  lines->GetCell(0, npts, pts);
  if(npts == 0)
    return;
  edgePoly->GetPoint(pts[0],p1);
  if(numCells > 1)
    {
    // get last cell
    lines->GetCell(numCells - 1, npts, pts);
    if(npts == 0)
      return;
    }
  edgePoly->GetPoint(pts[npts - 1], p2);
  this->ClosedLoop = p1[0] == p2[0]
                     && p1[1] == p2[1]
                     && p1[2] == p2[2];
  //get the number of points on the edge
  this->NumberOfPoints = edgePoly->GetNumberOfPoints();

  if(this->SelectedPointId >=0 && this->SelectedPointId < this->NumberOfPoints)
    {
    edgePoly->GetPoint(this->SelectedPointId, this->SelectedPointCoordinates);
    }
}

//----------------------------------------------------------------------------
void vtkPolygonArcInfo::CopyToStream(vtkClientServerStream* css)
{
  css->Reset();
  *css << vtkClientServerStream::Reply;
  *css << this->ClosedLoop
       << this->NumberOfPoints
       << this->SelectedPointId
       << vtkClientServerStream::InsertArray(this->SelectedPointCoordinates, 3)
       << strlen(this->ModelEntityID)
       << this->ModelEntityID
       << vtkClientServerStream::End;
}

//----------------------------------------------------------------------------
void vtkPolygonArcInfo::CopyFromStream(const vtkClientServerStream* css)
{
  css->GetArgument(0, 0, &this->ClosedLoop);
  css->GetArgument(0, 1, &this->NumberOfPoints);
  css->GetArgument(0, 2, &this->SelectedPointId);
  css->GetArgument(0, 3, this->SelectedPointCoordinates, 3);

  int len;
  css->GetArgument(0, 4, &len);
  this->SetModelEntityID(NULL);
  this->ModelEntityID = new char[len];
  css->GetArgument(0, 5, this->ModelEntityID, len);
}
