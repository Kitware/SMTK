/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
#include "vtkCmbMeshPolyDataProvider.h"

#include <vtkDiscreteModel.h>
#include "vtkCellArray.h"
#include "vtkCmbMeshServer.h"
#include "vtkCmbMeshWrapper.h"
#include "vtkCmbModelEntityMesh.h"
#include "vtkIdList.h"
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include "vtkModelEdge.h"
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkStreamingDemandDrivenPipeline.h>

vtkCxxRevisionMacro(vtkCmbMeshPolyDataProvider, "");
vtkStandardNewMacro(vtkCmbMeshPolyDataProvider);
vtkCxxSetObjectMacro(vtkCmbMeshPolyDataProvider, MeshWrapper, vtkCmbMeshWrapper);

//----------------------------------------------------------------------------
vtkCmbMeshPolyDataProvider::vtkCmbMeshPolyDataProvider()
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

//----------------------------------------------------------------------------
vtkCmbMeshPolyDataProvider::~vtkCmbMeshPolyDataProvider()
{
  this->SetMeshWrapper(NULL);
}

//----------------------------------------------------------------------------
void vtkCmbMeshPolyDataProvider::SetItemType(int itemType)
{
  this->ItemType = itemType;
  this->ItemTypeIsSet = 1;
}

//----------------------------------------------------------------------------
void vtkCmbMeshPolyDataProvider::SetEntityId(vtkIdType Id)
{
  this->EntityId = Id;
  this->EntityIdIsSet = 1;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCmbMeshPolyDataProvider::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "MeshWrapper: " << this->MeshWrapper << endl;
  os << indent << "ItemType: " << this->ItemType << endl;
  os << indent << "ItemTypeIsSet: " << this->ItemTypeIsSet << endl;
  os << indent << "EntityId: " << this->EntityId << endl;
  os << indent << "EntityIdIsSet: " << this->EntityIdIsSet << endl;
  os << indent << "CreateEdgePointVerts: " << this->CreateEdgePointVerts << endl;
}

//----------------------------------------------------------------------------
int vtkCmbMeshPolyDataProvider::RequestUpdateExtent(
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
int vtkCmbMeshPolyDataProvider::RequestData(
  vtkInformation* request,
  vtkInformationVector** vtkNotUsed( inputVector ),
  vtkInformationVector* outputVector)
{
  if(!this->MeshWrapper ||
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

  vtkModelEntity* entity = NULL;
  vtkModel* model = this->MeshWrapper->GetMesh()->GetModel();
  if(this->ItemTypeIsSet)
    {
    // the faster search method
    entity = model->GetModelEntity(this->ItemType, this->EntityId);
    }
  else
    {
    entity = model->GetModelEntity(this->EntityId);
    }
  vtkModelGeometricEntity* geometricEntity =
    vtkModelGeometricEntity::SafeDownCast(entity);
  if(geometricEntity == NULL)
    {
    vtkErrorMacro("Must have a geometric entity.");
    return 0;
    }
  vtkCmbModelEntityMesh* modelEntityMesh =
    this->MeshWrapper->GetMesh()->GetModelEntityMesh(geometricEntity);
  if(!modelEntityMesh->GetModelEntityMesh())
    {
    vtkWarningMacro("Input does not have valid geometry.");
    return 0;
    }

  // If this is an meshed model edge, we want to add vertex to the mesh polydata,
  // so that the mesh edge points can be shown.
  vtkModelEdge* EdgeEntity = vtkModelEdge::SafeDownCast(entity);
  if(EdgeEntity && this->CreateEdgePointVerts)
    {
    vtkPolyData* edgePoly = modelEntityMesh->GetModelEntityMesh();
    vtkIdType numCells = edgePoly->GetNumberOfCells();
    vtkIdType npts, *cellPnts;
    vtkSmartPointer<vtkIdList> PointIds =
      vtkSmartPointer<vtkIdList>::New();
    for(vtkIdType i=0; i<numCells; i++)
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
    for(vtkIdType i=0;i<NumberOfPointIds;i++)
      {
      vtkIdType PointId = PointIds->GetId(i);
      Verts->InsertNextCell(1, &PointId);
      }
    output->Initialize();
    output->SetPoints(edgePoly->GetPoints());
    output->SetLines(edgePoly->GetLines());
    if(Verts->GetNumberOfCells()>0)
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

//----------------------------------------------------------------------------
int vtkCmbMeshPolyDataProvider::FillOutputPortInformation(
  int, vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}
