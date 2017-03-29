//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBPolyDataProvider.h"

#include "vtkCellArray.h"
#include "vtkCommand.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTrivialProducer.h"

vtkStandardNewMacro(vtkCMBPolyDataProvider);
vtkCxxSetObjectMacro(vtkCMBPolyDataProvider, ModelWrapper, vtkDiscreteModelWrapper);

//----------------------------------------------------------------------------
vtkCMBPolyDataProvider::vtkCMBPolyDataProvider()
{
  // by default assume filters have one input and one output
  // subclasses that deviate should modify this setting
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
  this->ModelWrapper = 0;
  this->ItemType = -1;
  this->ItemTypeIsSet = 0;
  this->EntityId = -1;
  this->EntityIdIsSet = 0;
  this->CreateEdgePointVerts = false;
}

//----------------------------------------------------------------------------
vtkCMBPolyDataProvider::~vtkCMBPolyDataProvider()
{
  this->SetModelWrapper(0);
}

//----------------------------------------------------------------------------
void vtkCMBPolyDataProvider::SetItemType(int itemType)
{
  this->ItemType = itemType;
  this->ItemTypeIsSet = 1;
}

//----------------------------------------------------------------------------
void vtkCMBPolyDataProvider::SetEntityId(vtkIdType Id)
{
  this->EntityId = Id;
  this->EntityIdIsSet = 1;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCMBPolyDataProvider::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ModelWrapper: " << this->ModelWrapper << endl;
  os << indent << "ItemType: " << this->ItemType << endl;
  os << indent << "ItemTypeIsSet: " << this->ItemTypeIsSet << endl;
  os << indent << "EntityId: " << this->EntityId << endl;
  os << indent << "EntityIdIsSet: " << this->EntityIdIsSet << endl;
  os << indent << "CreateEdgePointVerts: " << this->CreateEdgePointVerts << endl;
}

//----------------------------------------------------------------------------
int vtkCMBPolyDataProvider::RequestUpdateExtent(
  vtkInformation *,
  vtkInformationVector **,
  vtkInformationVector *outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  int piece, numPieces, ghostLevel;

  piece = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER());
  numPieces = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());
  ghostLevel = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS());

  // make sure piece is valid
  if (piece < 0 || piece >= numPieces)
    {
    return 1;
    }

  if (ghostLevel < 0)
    {
    return 1;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkCMBPolyDataProvider::RequestData(
  vtkInformation* /*request*/,
  vtkInformationVector** vtkNotUsed( inputVector ),
  vtkInformationVector* outputVector)
{
  if(!this->ModelWrapper ||
     (this->EntityIdIsSet==0 && this->ItemTypeIsSet == 0) ||
     (this->EntityIdIsSet == 0 && this->ItemTypeIsSet != 0 &&
      this->ItemType != vtkModelType) )
    {
    vtkErrorMacro("Improper input to filter.");
    return 0;
    }

  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkModelEntity* Entity;
  vtkDiscreteModel* Model = this->ModelWrapper->GetModel();
  const DiscreteMesh& MasterGrid = Model->GetMesh();

  if(this->ItemTypeIsSet && this->ItemType == vtkModelType)
    {
    if(!MasterGrid.IsValid())
      {
      vtkWarningMacro("Could not get CMBModel polydata.");
      return 0;
      }

    //polydata represent the whole model
    //so we shallow copy
    output->ShallowCopy(MasterGrid.ShallowCopyFaceData());
    }
  else
    {
    if(this->ItemTypeIsSet)
      {
      // the faster search method
      Entity = Model->GetModelEntity(this->ItemType, this->EntityId);
      }
    else
      {
      Entity = this->ModelWrapper->GetModel()->GetModelEntity(this->EntityId);
      }
    vtkModelGeometricEntity* GeomEntity = vtkModelGeometricEntity::SafeDownCast(Entity);
    if(!GeomEntity)
      {
      vtkWarningMacro("Input is for a non-geometric model entity.");
      return 0;
      }
    if(!GeomEntity->GetGeometry())
      {
      vtkWarningMacro("Input does not have valid geometry.");
      return 0;
      }

    // If this is an model edge, we want to add vertex to the polydata, so that
    // the edge points can be shown.
    vtkDiscreteModelEdge* EdgeEntity = vtkDiscreteModelEdge::SafeDownCast(Entity);
     // model edge with vertexes for points
    if(EdgeEntity && this->CreateEdgePointVerts)
      {
      vtkDiscreteModelVertex* vertex1 = vtkDiscreteModelVertex::SafeDownCast(
        EdgeEntity->GetAdjacentModelVertex(0));
      vtkDiscreteModelVertex* vertex2 = vtkDiscreteModelVertex::SafeDownCast(
        EdgeEntity->GetAdjacentModelVertex(1));
      vtkSmartPointer<vtkIdList> VertexPointIds =
        vtkSmartPointer<vtkIdList>::New();
      if(vertex1)
        {
        VertexPointIds->InsertUniqueId(vertex1->GetPointId());
        }
      if(vertex2)
        {
        VertexPointIds->InsertUniqueId(vertex2->GetPointId());
        }
      vtkPolyData* edgePoly = vtkPolyData::SafeDownCast(GeomEntity->GetGeometry());
      vtkIdType numCells = edgePoly->GetNumberOfCells();
      vtkIdType npts, *cellPnts;
      vtkSmartPointer<vtkIdList> PointIds =
        vtkSmartPointer<vtkIdList>::New();
      for(vtkIdType i=0; i<numCells; i++)
        {
        edgePoly->GetCellPoints(i, npts, cellPnts);
        for (vtkIdType ptIndex = 0; ptIndex < npts; ptIndex++)
          {
          vtkIdType pid = cellPnts[ptIndex];
          if(VertexPointIds->IsId(pid)<0)
            {
            PointIds->InsertUniqueId(pid);
            }
          }
        }
      vtkIdType NumberOfPointIds = PointIds->GetNumberOfIds();
      vtkCellArray* Verts = vtkCellArray::New();
      Verts->Allocate(NumberOfPointIds);
      for(vtkIdType i=0;i<NumberOfPointIds;i++)
        {
        vtkIdType PointId = PointIds->GetId(i);
        Verts->InsertNextCell(1, &PointId);
        }
      output->Initialize();
      output->SetPoints(MasterGrid.SharePointsPtr());
      output->SetLines(vtkPolyData::SafeDownCast(GeomEntity->GetGeometry())->GetLines());
      if(Verts->GetNumberOfCells()>0)
        {
        output->SetVerts(Verts);
        }
      Verts->Delete();
      EdgeEntity->SetLineAndPointsGeometry(
        this->CreateEdgePointVerts ? output : NULL);
      }
    else
      {
      output->ShallowCopy(vtkDataObject::SafeDownCast(GeomEntity->GetGeometry()));
      output->SetPoints(MasterGrid.SharePointsPtr());
      }
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkCMBPolyDataProvider::FillOutputPortInformation(
  int, vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}

