/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
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
#include "vtkDiscreteModelFace.h"

#include "vtkBitArray.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModel.h"
#include "vtkConnectivityFilter.h"
#include "vtkFeatureEdges.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkSerializer.h"
#include "vtkModelFaceUse.h"
#include "vtkModelItemIterator.h"
#include "vtkModelShellUse.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"
#include "vtkSmartPointer.h"
#include "vtkSplitEventData.h"
#include <map>

vtkCxxRevisionMacro(vtkDiscreteModelFace, "");

vtkDiscreteModelFace* vtkDiscreteModelFace::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkDiscreteModelFace");
  if(ret)
    {
    return static_cast<vtkDiscreteModelFace*>(ret);
    }
  return new vtkDiscreteModelFace;
}

vtkDiscreteModelFace::vtkDiscreteModelFace()
{
}

vtkDiscreteModelFace::~vtkDiscreteModelFace()
{
}

vtkModelEntity* vtkDiscreteModelFace::GetThisModelEntity()
{
  return this;
}

bool vtkDiscreteModelFace::Split(
  double splitAngle, vtkIdTypeArray* createdModelFaces)
{
  createdModelFaces->Reset();
  createdModelFaces->SetNumberOfComponents(1);
  createdModelFaces->SetNumberOfTuples(0);
  vtkObject* geometry = this->GetGeometry();
  vtkPolyData* poly = vtkPolyData::SafeDownCast(geometry);

  if(poly == 0)
    {
    if(geometry)
      {
      // we are on the server...
      return 0;
      }
    // BoundaryRep has not been set -> return error
    return 0;
    }

  // on server, go ahead and perform the split
  vtkNew<vtkPolyDataNormals> pdNormals;
  pdNormals->SetFeatureAngle(splitAngle);
  pdNormals->SetInputData(0, poly);

  vtkNew<vtkConnectivityFilter> pdConnectivity;
  pdConnectivity->SetInputConnection(pdNormals->GetOutputPort());
  pdConnectivity->SetColorRegions(1);
  pdConnectivity->Update();

  vtkIdTypeArray* newFaceTags = vtkIdTypeArray::SafeDownCast(
    vtkDataSet::SafeDownCast(pdConnectivity->GetOutputDataObject(0))
    ->GetCellData()->GetArray("RegionId"));

  // filter the output
  // actually, newModelFaces also contains the old/source model face

  // the ids in newModelFaces are with respect to the master poly data
  std::map<vtkIdType, vtkSmartPointer<vtkIdList> > newModelFaces;

  vtkIdTypeArray* masterGeometryCellIndex = this->GetReverseClassificationArray();
  for(vtkIdType i=0;i<newFaceTags->GetNumberOfTuples();i++)
    {
    vtkIdType tag = newFaceTags->GetValue(i);
    if(newModelFaces.find(tag) == newModelFaces.end())
      {
      newModelFaces[tag] = vtkSmartPointer<vtkIdList>::New();
      }
    newModelFaces[tag]->InsertNextId(masterGeometryCellIndex->GetValue(i));
    }

  createdModelFaces->SetNumberOfTuples(newModelFaces.size()-1);

  if(newModelFaces.size() > 1)
    {
    vtkSplitEventData* splitEventData = vtkSplitEventData::New();
    splitEventData->SetSourceEntity(this);
    vtkIdList* newFaceIds = vtkIdList::New();
    newFaceIds->SetNumberOfIds(newModelFaces.size()-1); // the first one is the original
    std::map<vtkIdType, vtkSmartPointer<vtkIdList> >::iterator mit=newModelFaces.begin();
    mit++; // skip the first model face as it is the old/source model face
    for(vtkIdType i=0;mit!=newModelFaces.end();mit++,i++)
      {
      vtkDiscreteModelFace* face =
        this->BuildFromExistingModelFace(mit->second);
      createdModelFaces->SetValue(i, face->GetUniquePersistentId());
      newFaceIds->SetId(i, face->GetUniquePersistentId());
      }
    splitEventData->SetCreatedModelEntityIds(newFaceIds);
    newFaceIds->Delete();
    this->GetModel()->InvokeModelGeometricEntityEvent(ModelGeometricEntitySplit,
                                                      splitEventData);
    splitEventData->Delete();
    }

  return 1;
}

vtkDiscreteModelFace* vtkDiscreteModelFace::BuildFromExistingModelFace(
  vtkIdList* masterCellIds)
{
  bool blockEvent = this->GetModel()->GetBlockModelGeometricEntityEvent();
  this->GetModel()->SetBlockModelGeometricEntityEvent(true);
  vtkDiscreteModelFace* newModelFace = vtkDiscreteModelFace::SafeDownCast(
    this->GetModel()->BuildModelFace(0, 0, 0));
  this->GetModel()->SetBlockModelGeometricEntityEvent(blockEvent);

  newModelFace->AddCellsToGeometry(masterCellIds);

  // put in the adjacencies for the model face use
  for(int i=0;i<2;i++)
    {
    vtkModelFaceUse* sourceFaceUse = this->GetModelFaceUse(i);
    vtkModelShellUse* sourceShellUse = sourceFaceUse->GetModelShellUse();
    vtkModelFaceUse* targetFaceUse = newModelFace->GetModelFaceUse(i);
    if(sourceShellUse)
      {
      sourceShellUse->AddModelFaceUse(targetFaceUse);
      }
    }

  // new model face will belong to same groups as old model face
  newModelFace->CopyModelEntityGroups(this);
  this->GetModel()->InvokeModelGeometricEntityEvent(
    ModelGeometricEntityCreated, newModelFace);
  for(int i=0;i<2;i++)
    {
    if(vtkModelRegion* region = newModelFace->GetModelRegion(i))
      {
      this->GetModel()->InvokeModelGeometricEntityEvent(
        ModelGeometricEntityBoundaryModified, region);
      }
    }
  return newModelFace;
}

void vtkDiscreteModelFace::GetAllPointIds(vtkIdList* ptsList)
{
  vtkBitArray* Points = vtkBitArray::New();
  this->GatherAllPointIdsMask(Points);
  for(vtkIdType i=0;i<Points->GetNumberOfTuples();i++)
    {
    if(Points->GetValue(i) != 0)
      {
      ptsList->InsertNextId(i);
      }
    }
  Points->Delete();
}

void vtkDiscreteModelFace::GetInteriorPointIds(vtkIdList* ptsList)
{
  vtkBitArray* allModelFacePoints = vtkBitArray::New();
  this->GatherAllPointIdsMask(allModelFacePoints);
  vtkBitArray* boundaryModelFacePoints = vtkBitArray::New();
  this->GatherBoundaryPointIdsMask(boundaryModelFacePoints);
  for(vtkIdType i=0;i<allModelFacePoints->GetNumberOfTuples();i++)
    {
    if(allModelFacePoints->GetValue(i) != 0 &&
      boundaryModelFacePoints->GetValue(i) == 0)
      {
      ptsList->InsertNextId(i);
      }
    }
  allModelFacePoints->Delete();
  boundaryModelFacePoints->Delete();
}

void vtkDiscreteModelFace::GetBoundaryPointIds(vtkIdList* ptsList)
{
  vtkBitArray* points = vtkBitArray::New();
  this->GatherBoundaryPointIdsMask(points);
  for(vtkIdType i=0;i<points->GetNumberOfTuples();i++)
    {
    if(points->GetValue(i) != 0)
      {
      ptsList->InsertNextId(i);
      }
    }
  points->Delete();
}

void vtkDiscreteModelFace::GatherAllPointIdsMask(vtkBitArray* points)
{
  vtkPointSet* grid = vtkPointSet::SafeDownCast(this->GetGeometry());
  if(!grid)
    {
    vtkWarningMacro("Cannot access model face's grid.");
    return;
    }
  // Since not all points in Grid's points are attached to cells, we
  // iterate over the cells and get their points to set them in Points.
  points->SetNumberOfComponents(1);
  points->SetNumberOfTuples(grid->GetNumberOfPoints());
  for(vtkIdType i=0;i<grid->GetNumberOfPoints();i++)
    {
    points->SetValue(i, 0);
    }
  vtkIdType numberOfCells = grid->GetNumberOfCells();
  vtkIdList* cellPoints = vtkIdList::New();
  for(vtkIdType i=0;i<numberOfCells;i++)
    {
    grid->GetCellPoints(i, cellPoints);
    for(vtkIdType j=0;j<cellPoints->GetNumberOfIds();j++)
      {
      vtkIdType id=cellPoints->GetId(j);
      points->SetValue(id, 1);
      }
    }
  cellPoints->Delete();
}

void vtkDiscreteModelFace::GatherBoundaryPointIdsMask(vtkBitArray* points)
{
  vtkPointSet* grid = vtkPointSet::SafeDownCast(this->GetGeometry());
  // add in an array of the original point ids to grid
  vtkIdTypeArray* originalPointIds = vtkIdTypeArray::New();
  originalPointIds->SetNumberOfComponents(1);
  originalPointIds->SetNumberOfTuples(grid->GetNumberOfPoints());
  for(vtkIdType i=0;i<grid->GetNumberOfPoints();i++)
    {
    originalPointIds->SetValue(i, i);
    }
  const char arrayName[] = "vtkModelNodalGroupPointIdArray";
  originalPointIds->SetName(arrayName);
  grid->GetPointData()->AddArray(originalPointIds);
  originalPointIds->Delete();

  vtkNew<vtkFeatureEdges> featureEdges;
  featureEdges->BoundaryEdgesOn();
  featureEdges->NonManifoldEdgesOff();
  featureEdges->ManifoldEdgesOff();
  featureEdges->FeatureEdgesOff();
  featureEdges->SetInputData(grid);
  featureEdges->Update();
  vtkPointSet* gridEdges = vtkPointSet::SafeDownCast(featureEdges->GetOutput(0));

  // Since not all points in GridEdges's points are attached to cells, we
  // iterate over the cells and get their points to set them in Points.
  originalPointIds = vtkIdTypeArray::SafeDownCast(
    gridEdges->GetPointData()->GetArray(arrayName));

  points->SetNumberOfComponents(1);
  points->SetNumberOfTuples(grid->GetNumberOfPoints());
  for(vtkIdType i=0;i<grid->GetNumberOfPoints();i++)
    {
    points->SetValue(i, 0);
    }
  vtkIdType numberOfCells = gridEdges->GetNumberOfCells();
  vtkNew<vtkIdList> cellPoints;
  for(vtkIdType i=0;i<numberOfCells;i++)
    {
    gridEdges->GetCellPoints(i, cellPoints.GetPointer());
    for(vtkIdType j=0;j<cellPoints->GetNumberOfIds();j++)
      {
      vtkIdType id=cellPoints->GetId(j);
      vtkIdType masterGridId = originalPointIds->GetValue(id);
      points->SetValue(masterGridId, 1);
      }
    }
  grid->GetPointData()->RemoveArray(arrayName);
}

void vtkDiscreteModelFace::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

bool vtkDiscreteModelFace::Destroy()
{
  if(this->Superclass::Destroy())
    {
    this->RemoveAllAssociations(vtkDiscreteModelEntityGroupType);
    this->RemoveAllAssociations(vtkModelNodalGroupType);
    this->Modified();
    return true;
    }
  return false;
}

void vtkDiscreteModelFace::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
