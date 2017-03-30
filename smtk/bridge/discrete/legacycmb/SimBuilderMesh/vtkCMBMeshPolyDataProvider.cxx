//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBMeshPolyDataProvider.h"

#include "vtkCMBMeshServer.h"
#include "vtkCMBMeshWrapper.h"
#include "vtkCMBModelEntityMesh.h"
#include "vtkCellArray.h"
#include "vtkIdList.h"
#include "vtkModelEdge.h"
#include <vtkDiscreteModel.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkStreamingDemandDrivenPipeline.h>

vtkStandardNewMacro(vtkCMBMeshPolyDataProvider);
vtkCxxSetObjectMacro(vtkCMBMeshPolyDataProvider, MeshWrapper, vtkCMBMeshWrapper);

vtkCMBMeshPolyDataProvider::vtkCMBMeshPolyDataProvider()
{
  // by default assume filters have one input and one output
  // subclasses that deviate should modify this setting
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
  this->MeshWrapper = NULL;
  this->ItemType = -1;
  this->ItemTypeIsSet = 0;
  this->EntityId = -1;
  this->EntityIdIsSet = 0;
  this->CreateEdgePointVerts = false;
}

vtkCMBMeshPolyDataProvider::~vtkCMBMeshPolyDataProvider()
{
  this->SetMeshWrapper(NULL);
}

void vtkCMBMeshPolyDataProvider::SetItemType(int itemType)
{
  this->ItemType = itemType;
  this->ItemTypeIsSet = 1;
}

void vtkCMBMeshPolyDataProvider::SetEntityId(vtkIdType Id)
{
  this->EntityId = Id;
  this->EntityIdIsSet = 1;
  this->Modified();
}

void vtkCMBMeshPolyDataProvider::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "MeshWrapper: " << this->MeshWrapper << endl;
  os << indent << "ItemType: " << this->ItemType << endl;
  os << indent << "ItemTypeIsSet: " << this->ItemTypeIsSet << endl;
  os << indent << "EntityId: " << this->EntityId << endl;
  os << indent << "EntityIdIsSet: " << this->EntityIdIsSet << endl;
  os << indent << "CreateEdgePointVerts: " << this->CreateEdgePointVerts << endl;
}

int vtkCMBMeshPolyDataProvider::RequestUpdateExtent(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

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

int vtkCMBMeshPolyDataProvider::RequestData(vtkInformation* /*request*/,
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  if (!this->MeshWrapper || (this->EntityIdIsSet == 0 && this->ItemTypeIsSet == 0) ||
    (this->EntityIdIsSet == 0 && this->ItemTypeIsSet != 0 && this->ItemType != vtkModelType))
  {
    vtkErrorMacro("Improper input to filter.");
    return 0;
  }

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkModelEntity* entity = NULL;
  vtkModel* model = this->MeshWrapper->GetMesh()->GetModel();
  if (this->ItemTypeIsSet)
  {
    // the faster search method
    entity = model->GetModelEntity(this->ItemType, this->EntityId);
  }
  else
  {
    entity = model->GetModelEntity(this->EntityId);
  }
  vtkModelGeometricEntity* geometricEntity = vtkModelGeometricEntity::SafeDownCast(entity);
  if (geometricEntity == NULL)
  {
    vtkErrorMacro("Must have a geometric entity.");
    return 0;
  }
  vtkCMBModelEntityMesh* modelEntityMesh =
    this->MeshWrapper->GetMesh()->GetModelEntityMesh(geometricEntity);
  if (!modelEntityMesh->GetModelEntityMesh())
  {
    vtkWarningMacro("Input does not have valid geometry.");
    return 0;
  }

  // If this is an meshed model edge, we want to add vertex to the mesh polydata,
  // so that the mesh edge points can be shown.
  vtkModelEdge* EdgeEntity = vtkModelEdge::SafeDownCast(entity);
  if (EdgeEntity && this->CreateEdgePointVerts)
  {
    vtkPolyData* edgePoly = modelEntityMesh->GetModelEntityMesh();
    vtkIdType numCells = edgePoly->GetNumberOfCells();
    vtkIdType npts, *cellPnts;
    vtkSmartPointer<vtkIdList> PointIds = vtkSmartPointer<vtkIdList>::New();
    for (vtkIdType i = 0; i < numCells; i++)
    {
      edgePoly->GetLines()->GetCell(i, npts, cellPnts);
      for (vtkIdType ptIndex = 0; ptIndex < npts; ptIndex++)
      {
        vtkIdType pid = cellPnts[ptIndex];
        PointIds->InsertUniqueId(pid);
      }
    }
    vtkIdType NumberOfPointIds = PointIds->GetNumberOfIds();
    vtkCellArray* Verts = vtkCellArray::New();
    Verts->Allocate(NumberOfPointIds);
    for (vtkIdType i = 0; i < NumberOfPointIds; i++)
    {
      vtkIdType PointId = PointIds->GetId(i);
      Verts->InsertNextCell(1, &PointId);
    }
    output->Initialize();
    output->SetPoints(edgePoly->GetPoints());
    output->SetLines(edgePoly->GetLines());
    if (Verts->GetNumberOfCells() > 0)
    {
      output->SetVerts(Verts);
    }
    Verts->Delete();
  }
  else
  {
    output->ShallowCopy(modelEntityMesh->GetModelEntityMesh());
  }

  return 1;
}

int vtkCMBMeshPolyDataProvider::FillOutputPortInformation(int, vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}
