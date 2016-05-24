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
#include "vtkDataObject.h"
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
  if (!mbds || this->BlockIndex < 0
      || this->BlockIndex >= mbds->GetNumberOfBlocks())
    {
    return;
    }
  vtkPolyData* edgePoly = vtkPolyData::SafeDownCast(mbds->GetBlock(this->BlockIndex));
  if(!edgePoly || edgePoly->GetNumberOfPoints() <= 1
     || !edgePoly->GetNumberOfCells()
     || !edgePoly->GetLines())
    {
    return;
    }

  if(mbds->HasMetaData(this->BlockIndex))
    {
    vtkInformation* cinfo = mbds->GetMetaData(this->BlockIndex);
    if(cinfo->Has(vtkModelMultiBlockSource::ENTITYID()))
      this->SetModelEntityID(cinfo->Get(vtkModelMultiBlockSource::ENTITYID()));
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
}

//----------------------------------------------------------------------------
void vtkPolygonArcInfo::CopyToStream(vtkClientServerStream* css)
{
  css->Reset();
  *css << vtkClientServerStream::Reply;
  *css << this->ClosedLoop
       << this->NumberOfPoints
       << strlen(this->ModelEntityID)
       << this->ModelEntityID
       << vtkClientServerStream::End;
}

//----------------------------------------------------------------------------
void vtkPolygonArcInfo::CopyFromStream(const vtkClientServerStream* css)
{
  css->GetArgument(0, 0, &this->ClosedLoop);
  css->GetArgument(0, 1, &this->NumberOfPoints);
  int len;
  css->GetArgument(0, 3, &len);
  this->SetModelEntityID(NULL);
  this->ModelEntityID = new char[len];
  css->GetArgument(0, 3, this->ModelEntityID, len);
}
