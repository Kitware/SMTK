//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkSeedGrowSelectionFilter.h"

#include "vtkCellData.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#
#include "vtkTriangle.h"
#include <queue>
#include <set>
#

//----------------------------------------------------------------------------
class vtkSeedGrowSelectionFilter::vtkInternal
{
public:
  // Description:
  // A set of visible model faces
  std::set<vtkIdType> ModelFaceIds; //FaceIds;

  // The last element of the input array is the negative (-1).
  void updateIDs(vtkIdType thearray[])
    {
    int i=0;
    //should ModelFaceIds get reset first?
    while (thearray[i] >= 0 && i < VTK_INT_MAX)
      {
      this->ModelFaceIds.insert(thearray[i]);
      i++;
      }
    }

  void setIDs(const std::set<vtkIdType>& faceIds)
    {
    this->ModelFaceIds.clear();
    this->ModelFaceIds.insert(faceIds.begin(), faceIds.end());
    }

  bool IsModelFaceVisible(vtkIdType modelFaceId)
    {
      if(this->ModelFaceIds.find(modelFaceId) ==
         this->ModelFaceIds.end())
        {
        return 0;
        }
      return 1;
    }

  bool IsModelFaceVisible(vtkDiscreteModelGeometricEntity* modelFace)
    {
      vtkModelEntity* entity = modelFace->GetThisModelEntity();
      vtkIdType modelFaceId = entity->GetUniquePersistentId();
      return this->IsModelFaceVisible(modelFaceId);
    }
  vtkNew<vtkSelection> InputSelection;
};

vtkStandardNewMacro(vtkSeedGrowSelectionFilter);
vtkCxxSetObjectMacro(vtkSeedGrowSelectionFilter, ModelWrapper, vtkDiscreteModelWrapper);

//----------------------------------------------------------------------------
vtkSeedGrowSelectionFilter::vtkSeedGrowSelectionFilter()
{
  this->FeatureAngle = 70.0;
  this->CellId = -1;
  this->ModelFaceId = -1;
  this->GrowMode = 0;
  this->SetNumberOfInputPorts(1);

  this->Internal = new vtkInternal();

  this->ModelWrapper = 0;
}

//----------------------------------------------------------------------------
vtkSeedGrowSelectionFilter::~vtkSeedGrowSelectionFilter()
{
  delete this->Internal;

  this->SetModelWrapper(0);
}

//----------------------------------------------------------------------------
void vtkSeedGrowSelectionFilter::SetFaceCellId(vtkIdType faceId, vtkIdType cellId)
{
  if(this->CellId == cellId && this->ModelFaceId == faceId)
    {
    return;
    }
  this->CellId = cellId;
  this->ModelFaceId = faceId;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkSeedGrowSelectionFilter::RemoveAllGrowFaceIds()
{
  this->Internal->ModelFaceIds.clear();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkSeedGrowSelectionFilter::SetGrowFaceIds(vtkIdType* faceIds)
{
  this->Internal->updateIDs(faceIds);

  this->Modified();
}
//----------------------------------------------------------------------------
void vtkSeedGrowSelectionFilter::SetGrowFaceIds(
  const std::set<vtkIdType>& faceIds)
{
  this->Internal->setIDs(faceIds);
  this->Modified();
}
//----------------------------------------------------------------------------
void vtkSeedGrowSelectionFilter::GrowFromCell(const DiscreteMesh *mesh, vtkIntArray* marked, vtkIdType inputCellId,
  vtkIdTypeArray* outSelectionList)
{
  vtkDiscreteModel* model = this->ModelWrapper->GetModel();
  vtkDiscreteModel::ClassificationType& classified =
                            model->GetMeshClassification();
  std::queue<vtkIdType> cellQueue;
  cellQueue.push(inputCellId);
  outSelectionList->InsertNextValue(inputCellId);

  vtkIdList* edgePts = vtkIdList::New();
  edgePts->SetNumberOfIds(2);
  vtkIdList* neighbors = vtkIdList::New();
  vtkIdList* pts = vtkIdList::New();
  vtkIdList* otherPts = vtkIdList::New();
  double normal[3];
  while(!cellQueue.empty())
    {
    vtkIdType cellId = cellQueue.front();
    cellQueue.pop();
    mesh->GetCellPointIds(cellId, pts);
    this->ComputeNormal(pts, mesh, normal);
    const int numberOfPoints = pts->GetNumberOfIds();
    for(int i=0;i<numberOfPoints;i++)
      {
      edgePts->SetId(0, pts->GetId(i));
      edgePts->SetId(1, pts->GetId((i+1)%numberOfPoints));
      mesh->GetCellNeighbors(cellId, edgePts, neighbors);
      // may have multiple edge neighbors for non-manifold mesh
      const vtkIdType numberOfNeighbors = neighbors->GetNumberOfIds();
      for(vtkIdType id=0;id<numberOfNeighbors;id++)
        {
        vtkIdType otherId = neighbors->GetId(id);
        // first check if marked
        if(marked->GetValue(otherId) == 0)
          {
          vtkDiscreteModelGeometricEntity* modelFace =
                                              classified.GetEntity(otherId);
          if(this->Internal->IsModelFaceVisible(modelFace))
            {
            // also needs to be visible in order to grow to the cell
            double otherNormal[3];
            mesh->GetCellPointIds(otherId, otherPts);
            this->ComputeNormal(otherPts, mesh, otherNormal);
            double dotprod = vtkMath::Dot(normal, otherNormal);
            if(!this->AreCellNormalsConsistent(
                 edgePts->GetId(0), edgePts->GetId(1), otherPts))
              {
              dotprod = -dotprod;
              }
            if(dotprod > this->FeatureAngleCosine )
              {
              marked->SetValue(otherId, 1);
              outSelectionList->InsertNextValue(otherId);
              cellQueue.push(otherId);
              }
            }
          }
        }
      }
    }
  edgePts->Delete();
  neighbors->Delete();
  pts->Delete();
  otherPts->Delete();
}

//----------------------------------------------------------------------------
void vtkSeedGrowSelectionFilter::GrowAndRemoveFromSelection(
  const DiscreteMesh *mesh, vtkIntArray* marked, vtkIdType inputCellId,
  vtkIdTypeArray* outSelectionList, vtkSelection* inSelection)
{
  if(!inSelection)
    {
    return;
    }
  vtkIdTypeArray* selIdArray = vtkIdTypeArray::SafeDownCast(
    inSelection->GetNode(0)->GetSelectionList());
  if(!selIdArray || selIdArray->GetNumberOfTuples()==0)
    {
    return;
    }
  vtkIdType inIdx = selIdArray->LookupValue(inputCellId);
  if(inIdx<0)
    {
    outSelectionList->DeepCopy(selIdArray);
    //this->MergeGrowSelection(selection, marked);
    return;
    }

  int numRemIds = 0;
  std::queue<vtkIdType> cellQueue;
  cellQueue.push(inputCellId);
  //selIdArray->RemoveTuple(inIdx);
  selIdArray->SetValue(inIdx, -1);
  numRemIds++;
  //outSelectionList->InsertNextValue(inputCellId);

  vtkIdList* edgePts = vtkIdList::New();
  edgePts->SetNumberOfIds(2);
  vtkIdList* neighbors = vtkIdList::New();
  vtkIdList* pts = vtkIdList::New();
  vtkIdList* otherPts = vtkIdList::New();
  double normal[3];
  while(!cellQueue.empty())
    {
    vtkIdType cellId = cellQueue.front();
    cellQueue.pop();
    mesh->GetCellPointIds(cellId, pts);
    this->ComputeNormal(pts, mesh, normal);
    const int numberOfPoints = pts->GetNumberOfIds();
    for(int i=0;i<numberOfPoints;i++)
      {
      edgePts->SetId(0, pts->GetId(i));
      edgePts->SetId(1, pts->GetId((i+1)%numberOfPoints));
      mesh->GetCellNeighbors(cellId, edgePts, neighbors);
      // may have multiple edge neighbors for non-manifold mesh
      const vtkIdType numberOfNeighbors = neighbors->GetNumberOfIds();
      for(vtkIdType id=0;id<numberOfNeighbors;id++)
        {
        vtkIdType otherId = neighbors->GetId(id);
        // first check if marked
        if(marked->GetValue(otherId) ==0)
          {
          inIdx = selIdArray->LookupValue(otherId);
          if(inIdx<0)
            {
            continue;
            }
          double otherNormal[3];
          mesh->GetCellPointIds(otherId, otherPts);
          this->ComputeNormal(otherPts, mesh, otherNormal);
          double dotprod = vtkMath::Dot(normal, otherNormal);
          if(!this->AreCellNormalsConsistent(
               edgePts->GetId(0), edgePts->GetId(1), otherPts))
            {
            dotprod = -dotprod;
            }
          if(dotprod > this->FeatureAngleCosine )
            {
            selIdArray->SetValue(inIdx, -1);
            numRemIds++;
            marked->SetValue(otherId, 1);
            //(*numCells)++;
            cellQueue.push(otherId);
            }
          }
        }
      }
    }

  int numSelIds = selIdArray->GetNumberOfTuples();
  if(numRemIds < numSelIds)
    {
    vtkIdType selId;
    for(int idx=0; idx<numSelIds; idx++)
      {
      selId = selIdArray->GetValue(idx);
      if( selId != -1)
        {
        outSelectionList->InsertNextValue(selId);
        }
      }
    }
  otherPts->Delete();
  edgePts->Delete();
  neighbors->Delete();
  pts->Delete();
}

//----------------------------------------------------------------------------
void vtkSeedGrowSelectionFilter::ComputeNormal(
  vtkIdList* ptids, const DiscreteMesh* mesh, double* normal)
{
  switch(ptids->GetNumberOfIds())
    {
    case 3:
    case 4:
    double pts0[3], pts1[3], pts2[3];
    mesh->GetPoint(ptids->GetId(0), pts0);
    mesh->GetPoint(ptids->GetId(1), pts1);
    mesh->GetPoint(ptids->GetId(2), pts2);
    vtkTriangle::ComputeNormal(pts0, pts1, pts2, normal);
    return;

    default:
    vtkIdType numberOfPoints = ptids->GetNumberOfIds();
    std::vector<double> pts(numberOfPoints*3);
    for(vtkIdType i=0;i<numberOfPoints;i++)
      {
      mesh->GetPoint(ptids->GetId(i), &(pts[3*i]));
      }
    vtkPolygon::ComputeNormal(numberOfPoints, &(pts[0]), normal);
    }
}

//----------------------------------------------------------------------------
bool vtkSeedGrowSelectionFilter::AreCellNormalsConsistent(
  vtkIdType pointId1, vtkIdType pointId2, vtkIdList * neighborCellIds)
{
  vtkIdType numberOfPoints = neighborCellIds->GetNumberOfIds();
  for(vtkIdType i=0;i<numberOfPoints;i++)
    {
    if(pointId1 == neighborCellIds->GetId(i) &&
       pointId2 == neighborCellIds->GetId((i+1)%numberOfPoints))
      {
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
void vtkSeedGrowSelectionFilter::SetSelectionConnection(
  vtkAlgorithmOutput* algOutput)
{
  this->SetInputConnection(0, algOutput);
}

//----------------------------------------------------------------------------
void vtkSeedGrowSelectionFilter::SetInputSelection(vtkSelection* inSelection)
{
  // if this is set, the selection from input connection will not be used.
  this->Internal->InputSelection->Initialize();
  if(inSelection)
    this->Internal->InputSelection->DeepCopy(inSelection);
}

//----------------------------------------------------------------------------
void vtkSeedGrowSelectionFilter::MergeGrowSelection(
  vtkSelection* selection, vtkIntArray* marked,
  vtkIdTypeArray* outSelectionList)
{
  vtkIdTypeArray* array = NULL;
  vtkSelectionNode* selNode = NULL;
  for(unsigned int ui=0;ui<selection->GetNumberOfNodes();ui++)
    {
    selNode = selection->GetNode(ui);
    array = vtkIdTypeArray::SafeDownCast(
      selNode->GetSelectionList());
    if(!array)
      {
      continue;
      }
    for(int i=0;i<array->GetNumberOfTuples();i++)
      {
      int selId = array->GetValue(i);;  // add in ids here to marked
      if(marked->GetValue(selId)==0)
        {
        // no need for this since marked is not used after this
        //marked->SetValue(selId, 1);
        outSelectionList->InsertNextValue(selId);
        }
      }
    }
}

//----------------------------------------------------------------------------
int vtkSeedGrowSelectionFilter::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  vtkSelection* output = vtkSelection::GetData(outputVector);
  vtkSelectionNode* selNode = vtkSelectionNode::New();
  vtkInformation* oProperties = selNode->GetProperties();
  oProperties->Set(vtkSelectionNode::CONTENT_TYPE(),
                   vtkSelectionNode::INDICES);
  oProperties->Set(vtkSelectionNode::FIELD_TYPE(), vtkSelectionNode::CELL);
  output->AddNode(selNode);
  selNode->Delete();

  vtkSelection* inSelection = this->Internal->InputSelection.GetPointer();

  if((!inSelection || inSelection->GetNumberOfNodes()==0)
    && this->GetNumberOfInputPorts() > 0 && this->GrowMode > 0)
    {
    vtkInformation *selInfo = inputVector[0]->GetInformationObject(0);
    if ( selInfo )
      {
      inSelection = vtkSelection::SafeDownCast(
        selInfo->Get(vtkDataObject::DATA_OBJECT()));
      }
    }

  if(!this->ModelWrapper)
    {
    vtkErrorMacro("Must set ModelWrapper.");
    return 0;
    }
  vtkDiscreteModel* Model = this->ModelWrapper->GetModel();
  if(Model->HasInValidMesh())
    {
    vtkErrorMacro("Could not get mesh from model.");
    return 0;
    }

  vtkIdType numCells = Model->GetMesh().GetNumberOfFaces();

  // the CellId we have now is the Id from the Model Face that it is
  // associated with, we need to get this cell's Id on the master poly data...
  // need to skip the root index for selection in composite data
  // unsigned int cur_index = static_cast<unsigned int>(this->ModelFaceCompositeIdx);
  vtkIdType faceId = this->ModelFaceId;
/*
  if (!this->ModelWrapper->GetEntityIdByChildIndex(cur_index, faceId))
    {
    vtkErrorMacro("Could not find the model face from the composite index." << this->ModelFaceCompositeIdx);
    return 0;
    }
*/
  vtkDiscreteModelFace* Face = vtkDiscreteModelFace::SafeDownCast(
    Model->GetModelEntity(vtkModelFaceType, faceId));
  if(!Face)
    {
    vtkErrorMacro("Could not get model face with Id " << faceId);
    return 0;
    }
  vtkIdType MasterCellId = Face->GetMasterCellId(this->CellId);
  if (MasterCellId<0 || MasterCellId>=numCells)
    {
    vtkErrorMacro("No cell found by given cellId and faceId!");
    return 0;
    }

  vtkIntArray* marked = vtkIntArray::New();
  marked->SetNumberOfComponents(1);
  marked->SetNumberOfTuples(numCells);
  int i;
  for(i=0;i<numCells;i++)
    {
    marked->SetValue(i, 0);
    }
  marked->SetValue(MasterCellId, 1);

  //get the vtkPolydata from the mesh
  const DiscreteMesh &mesh = Model->GetMesh();

  vtkIdList* pts = vtkIdList::New();
  mesh.GetCellPointIds(MasterCellId, pts);
  double normal[3];
  this->ComputeNormal(pts, &mesh, normal);
  this->FeatureAngleCosine = cos(
    vtkMath::RadiansFromDegrees(this->FeatureAngle));

  // Create the selection list
  vtkIdTypeArray* outSelectionList = vtkIdTypeArray::New();
  outSelectionList->SetNumberOfComponents(1);

  if(this->GrowMode == 0 || this->GrowMode == 1) // Grow and/or Merge
    {
    this->GrowFromCell(&mesh, marked, MasterCellId, outSelectionList);
    if(inSelection && this->GrowMode == 1) // Merge
      {
      this->MergeGrowSelection(inSelection, marked, outSelectionList);
      }
    }
  else if (this->GrowMode == 2) // grow and remove
    {
    this->GrowAndRemoveFromSelection(&mesh, marked, MasterCellId,
                                     outSelectionList, inSelection);
    }

  selNode->SetSelectionList(outSelectionList);
  outSelectionList->Delete();

  oProperties->Set(vtkSelectionNode::CONTAINING_CELLS(),1);

  oProperties->Set(vtkSelectionNode::INVERSE(),0);

  if (selNode->GetSelectionList())
    {
    selNode->GetSelectionList()->SetName("IDs");
    }

  pts->Delete();
  marked->Delete();

  return 1;
}

//----------------------------------------------------------------------------
int vtkSeedGrowSelectionFilter::FillInputPortInformation(
  int /*port*/, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkSelection");
  info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);

  return 1;
}

//----------------------------------------------------------------------------
void vtkSeedGrowSelectionFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FeatureAngle: " << this->FeatureAngle << "\n";
  os << indent << "CellId: " << this->CellId << "\n";
  os << indent << "ModelFaceId: " << this->ModelFaceId << "\n";
  os << indent << "GrowMode: " << this->GrowMode << "\n";
}

