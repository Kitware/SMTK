//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBIncorporateMeshOperator.h"

#include "vtkAlgorithm.h"
#include "vtkCMBModelBuilder.h"
#include "vtkCellArray.h"
#include "vtkDataSet.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkFieldData.h"
#include "vtkIdList.h"
#include "vtkIncrementalOctreePointLocator.h"
#include "vtkMasterPolyDataNormals.h"
#include "vtkMergeDuplicateCells.h"
#include "vtkModelItemIterator.h"
#include "vtkModelMaterial.h"
#include "vtkModelUserName.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkStringArray.h"

#include "ModelEdgeHelper.h"
#include "ModelParserHelper.h"
#include <set>
#include <vtkNew.h>

vtkStandardNewMacro(vtkCMBIncorporateMeshOperator);

vtkCMBIncorporateMeshOperator::vtkCMBIncorporateMeshOperator()
{
  this->OperateSucceeded = 0;
}

vtkCMBIncorporateMeshOperator::~vtkCMBIncorporateMeshOperator()
{
  this->ClearSolidMeshes();
}

void vtkCMBIncorporateMeshOperator::Operate(vtkDiscreteModelWrapper* modelWrapper)
{
  vtkDebugMacro("Incorporating solid meshes into a CMB model.");
  this->OperateSucceeded = 0;
  if (this->SolidMeshes.size() == 0)
  {
    vtkWarningMacro("There is no solid mesh input.");
    return;
  }

  if (!modelWrapper)
  {
    vtkErrorMacro("Passed in a null model.");
    return;
  }
  vtkDiscreteModel* model = modelWrapper->GetModel();
  if (model->GetModelDimension() == 2)
  {
    vtkErrorMacro("Currently doesnot support 2D model.");
    return;
  }

  std::map<vtkIdType, vtkPolyData*>::iterator it;
  for (it = this->SolidMeshes.begin(); it != this->SolidMeshes.end(); ++it)
  {
    this->IncorporateSolidMesh(model, (*it).first, (*it).second);
  }
  this->OperateSucceeded = 1;

  return;
}

void vtkCMBIncorporateMeshOperator::AddSolidMesh(
  vtkIdType meshRegionId, vtkPolyData* solidRegionSurface)
{
  if (solidRegionSurface)
  {
    vtkStringArray* solidFileName = vtkStringArray::SafeDownCast(
      solidRegionSurface->GetFieldData()->GetAbstractArray("FileName"));
    // The solid mesh file has to exist
    if (solidFileName)
    {
      if (this->SolidMeshes.find(meshRegionId) == this->SolidMeshes.end())
      {
        this->SolidMeshes[meshRegionId] = solidRegionSurface;
      }
    }
  }
}

void vtkCMBIncorporateMeshOperator::AddSolidMesh(vtkIdType meshRegionId, vtkAlgorithm* algOut)
{
  if (algOut)
  {
    vtkPolyData* meshOut = vtkPolyData::SafeDownCast(algOut->GetOutputDataObject(0));
    if (meshOut)
    {
      this->AddSolidMesh(meshRegionId, meshOut);
    }
  }
}

void vtkCMBIncorporateMeshOperator::ClearSolidMeshes()
{
  this->SolidMeshes.clear();
}

bool vtkCMBIncorporateMeshOperator::IncorporateSolidMesh(
  vtkDiscreteModel* meshModel, const vtkIdType& meshRegionId, vtkPolyData* solidRegionSurface)
{
  vtkStringArray* solidFileName =
    vtkStringArray::SafeDownCast(solidRegionSurface->GetFieldData()->GetAbstractArray("FileName"));
  if (!solidFileName)
  {
    return false;
  }
  std::string solidFile = solidFileName->GetValue(0);

  // master polydata normals (make sure each region has normals pointing out)
  vtkNew<vtkMasterPolyDataNormals> masterPolyDataNormals;
  masterPolyDataNormals->SetInputData(solidRegionSurface);
  masterPolyDataNormals->Update();

  //Merge duplicate cells filter
  vtkNew<vtkMergeDuplicateCells> mergeDuplicateCells;
  mergeDuplicateCells->SetInputData(masterPolyDataNormals->GetOutput());
  mergeDuplicateCells->SetModelRegionArrayName(ModelParserHelper::GetShellTagName());
  mergeDuplicateCells->SetModelFaceArrayName(ModelParserHelper::GetModelFaceTagName());
  mergeDuplicateCells->Update();

  vtkNew<vtkCMBModelBuilder> builderOperator;
  vtkNew<vtkDiscreteModelWrapper> modelWrapper;
  builderOperator->Operate(modelWrapper.GetPointer(), mergeDuplicateCells.GetPointer());
  if (!builderOperator->GetOperateSucceeded())
  {
    return false;
  }
  vtkDiscreteModel* solidModel = modelWrapper->GetModel();
  if (solidModel->GetModelDimension() == 2) // there is no vtkModelRegionType
  {
    return false;
  }

  vtkDiscreteModelRegion* meshRegion = vtkDiscreteModelRegion::SafeDownCast(
    meshModel->GetModelEntity(vtkModelRegionType, meshRegionId));
  if (!meshRegion)
  {
    return false;
  }
  int numRegions = solidModel->GetNumberOfModelEntities(vtkModelRegionType);
  if (numRegions == 1)
  {
    // All we need to do here is to set the solid file name.
    meshRegion->SetSolidFileName(solidFile.c_str());
    return true;
  }

  // If we have more than one region in the solid model, we need to
  // incorporate solidModel with current mesh model
  return this->SplitMeshRegion(meshRegion, solidModel, solidFile);
}

bool vtkCMBIncorporateMeshOperator::SplitMeshRegion(
  vtkDiscreteModelRegion* meshRegion, vtkDiscreteModel* solidModel, const std::string& solidFile)
{
  const DiscreteMesh& mesh = solidModel->GetMesh();
  mesh.BuildLinks();
  vtkSmartPointer<vtkIncrementalOctreePointLocator> locator =
    mesh.BuildPointLocator(DiscreteMesh::FACE_DATA);
  vtkIdType numSolidPts = mesh.GetNumberOfPoints();

  std::map<vtkDiscreteModelRegion*, std::set<vtkDiscreteModelFace*> > SolidRegionMeshFacesMap;
  std::set<vtkDiscreteModelFace*> UsedSolidFaces;
  std::map<vtkIdType, vtkIdType> SolidToMeshPointMap;
  vtkNew<vtkIdList> SolidCellMask;
  vtkModelItemIterator* iterMeshFace = meshRegion->NewAdjacentModelFaceIterator();
  for (iterMeshFace->Begin(); !iterMeshFace->IsAtEnd(); iterMeshFace->Next())
  {
    vtkDiscreteModelFace* meshFace =
      vtkDiscreteModelFace::SafeDownCast(iterMeshFace->GetCurrentItem());
    if (!meshFace)
    {
      vtkWarningMacro("Not a valid model face ??") continue;
    }
    std::map<vtkDiscreteModelRegion*, vtkSmartPointer<vtkIdList> > SolidRegionMeshCellsMap;
    vtkPolyData* meshPoly = vtkPolyData::SafeDownCast(meshFace->GetGeometry());
    vtkIdType i, n = meshPoly->GetNumberOfCells();
    vtkIdType cellId = -1, ptId = -1, pid;
    vtkNew<vtkIdList> cellPnts;
    bool ptFound;
    double p[3];
    double minDis;
    for (i = 0; i < n; i++)
    {
      meshPoly->GetCellPoints(i, cellPnts.GetPointer());
      ptFound = false;
      for (vtkIdType ptIndex = 0; ptIndex < cellPnts->GetNumberOfIds(); ptIndex++)
      {
        pid = cellPnts->GetId(ptIndex);
        meshPoly->GetPoint(pid, p);
        ptId = locator->FindClosestPointWithinSquaredRadius(0.001, p, minDis);
        // We need some tolerance here, since the points
        // may be changed during transform calculation.

        if (minDis < 1e-10 && ptId >= 0 && ptId < numSolidPts)
        {
          ptFound = true;
          break;
        }
        else
        {
          vtkWarningMacro("Missing a point ??");
        }
      }
      if (ptFound)
      {
        vtkDiscreteModel::ClassificationType& classified = solidModel->GetMeshClassification();

        // if a point is found on solid,
        // find the corresponding cell on the solid
        cellId = this->FindPointsCell(&mesh, ptId, meshPoly, cellPnts.GetPointer(),
          SolidCellMask.GetPointer(), SolidToMeshPointMap);
        if (cellId != -1)
        {
          SolidCellMask->InsertNextId(cellId);
          vtkDiscreteModelGeometricEntity* geoEntity = classified.GetEntity(cellId);
          if (geoEntity)
          {
            vtkDiscreteModelFace* solidFace =
              vtkDiscreteModelFace::SafeDownCast(geoEntity->GetThisModelEntity());
            if (solidFace && solidFace->GetNumberOfModelRegions() == 1)
            {
              // this faces should only have one region
              vtkDiscreteModelRegion* solRegion =
                vtkDiscreteModelRegion::SafeDownCast(solidFace->GetModelRegion(0));
              if (SolidRegionMeshCellsMap.find(solRegion) == SolidRegionMeshCellsMap.end())
              {
                vtkSmartPointer<vtkIdList> cellList = vtkSmartPointer<vtkIdList>::New();
                SolidRegionMeshCellsMap[solRegion] = cellList;
              }
              SolidRegionMeshCellsMap[solRegion]->InsertNextId(meshFace->GetMasterCellId(i));
              UsedSolidFaces.insert(solidFace);
            }
            else
            {
              vtkWarningMacro("The solid face found is not correct ??");
            }
          }
          else
          {
            vtkWarningMacro("Did not find solid entity given a cell ??");
          }
        }
        else
        {
          vtkWarningMacro("Missing a cell ??");
        }
      } // end if(ptFound)
    }   // end for each cell

    if (SolidRegionMeshCellsMap.size() > 0)
    {
      std::map<vtkDiscreteModelRegion*, vtkSmartPointer<vtkIdList> >::iterator itReg =
        SolidRegionMeshCellsMap.begin();
      // skip the first region
      SolidRegionMeshFacesMap[(*itReg).first].insert(meshFace);
      vtkDiscreteModelFace* newMeshFace;
      FaceEdgeSplitInfo dummyInfo;
      for (++itReg; itReg != SolidRegionMeshCellsMap.end(); itReg++)
      {
        // we need split the mesh face for each extra solid region
        newMeshFace = meshFace->BuildFromExistingModelFace((*itReg).second, dummyInfo, false);
        if (newMeshFace)
        {
          SolidRegionMeshFacesMap[(*itReg).first].insert(newMeshFace);
        }
      }
    }
  } // end of each mesh face
  iterMeshFace->Delete();
  return this->CreateNewMeshRegions(solidModel, meshRegion, SolidRegionMeshFacesMap, solidFile,
    SolidToMeshPointMap, UsedSolidFaces);
}

bool vtkCMBIncorporateMeshOperator::CreateNewMeshRegions(vtkDiscreteModel* solidModel,
  vtkDiscreteModelRegion* meshRegion,
  std::map<vtkDiscreteModelRegion*, std::set<vtkDiscreteModelFace*> >& SolidRegionMeshFacesMap,
  const std::string& solidFile, std::map<vtkIdType, vtkIdType>& SolidToMeshPointMap,
  std::set<vtkDiscreteModelFace*>& UsedSolidFaces)
{

  vtkDiscreteModel* meshModel = vtkDiscreteModel::SafeDownCast(meshRegion->GetModel());
  // we need a map for matching new solid faces with mesh faces.
  // The map only contains those solid faces that are not
  // covered by UsedSolidFaces, ie, the internal faces of the solid
  std::map<vtkDiscreteModelFace*, vtkDiscreteModelFace*> InnerSolFaceToMeshFaceMap;
  vtkModelItemIterator* iterSolReg = solidModel->NewIterator(vtkModelRegionType);
  int numRegion = 0;
  for (iterSolReg->Begin(); !iterSolReg->IsAtEnd(); iterSolReg->Next())
  {
    vtkDiscreteModelRegion* entRegion =
      vtkDiscreteModelRegion::SafeDownCast(iterSolReg->GetCurrentItem());
    if (!entRegion)
    {
      vtkWarningMacro("Not a valid model region in solid ??") continue;
    }
    vtkModelMaterial* newMaterial = meshModel->BuildMaterial();
    std::vector<vtkModelFace*> Faces;
    std::vector<int> FaceSides;

    if (SolidRegionMeshFacesMap.find(entRegion) != SolidRegionMeshFacesMap.end())
    {
      // All these meshes faces should already been split, and also been covered
      // by UsedSolidFaces, we just need to add these faces to the new regions
      std::set<vtkDiscreteModelFace*>::iterator itFace;
      for (itFace = SolidRegionMeshFacesMap[entRegion].begin();
           itFace != SolidRegionMeshFacesMap[entRegion].end(); ++itFace)
      {
        vtkModelFace* face = (*itFace);
        int side = (face->GetModelRegion(0) == meshRegion) ? 0 : 1;
        Faces.push_back(face);
        FaceSides.push_back(side);
      }
    }

    vtkModelItemIterator* iterFace = entRegion->NewAdjacentModelFaceIterator();
    for (iterFace->Begin(); !iterFace->IsAtEnd(); iterFace->Next())
    {
      vtkDiscreteModelFace* faceEntity =
        vtkDiscreteModelFace::SafeDownCast(iterFace->GetCurrentItem());
      if (!faceEntity)
      {
        vtkWarningMacro("Not a valid model face ??") continue;
      }
      vtkDiscreteModelFace* newFace = NULL;
      if (UsedSolidFaces.find(faceEntity) == UsedSolidFaces.end())
      {
        // Brand new solid face.
        // an internal face of the solid, which should be
        // used to create the new mesh region.
        // we need to create new mesh faces for the mesh
        newFace =
          vtkDiscreteModelFace::SafeDownCast(meshModel->BuildModelFace(0, 0, 0, newMaterial));
        // Add new cells to mesh master polydata
        vtkNew<vtkIdList> newCellIds;
        this->AddSolidFaceCells(
          meshModel, faceEntity, newCellIds.GetPointer(), SolidToMeshPointMap);
        newFace->AddCellsToGeometry(newCellIds.GetPointer());
        UsedSolidFaces.insert(faceEntity);
        InnerSolFaceToMeshFaceMap[faceEntity] = newFace;
      }
      else if (InnerSolFaceToMeshFaceMap.find(faceEntity) != InnerSolFaceToMeshFaceMap.end())
      {
        newFace = InnerSolFaceToMeshFaceMap[faceEntity];
      }
      if (newFace)
      {
        int side = (faceEntity->GetModelRegion(0) == entRegion) ? 0 : 1;
        Faces.push_back(newFace);
        FaceSides.push_back(side);
      }
    }
    iterFace->Delete();

    size_t NumFaces = Faces.size();
    if (NumFaces > 0)
    {
      numRegion++;
      char newRegionName[256];
      std::string regName = vtkModelUserName::GetUserName(meshRegion);
      sprintf(newRegionName, "%s_%d", regName.c_str(), numRegion);
      vtkDiscreteModelRegion* newRegion =
        vtkDiscreteModelRegion::SafeDownCast(meshModel->BuildModelRegion(
          static_cast<int>(NumFaces), &Faces[0], &FaceSides[0], newMaterial));
      newRegion->SetSolidFileName(solidFile.c_str());
      vtkModelUserName::SetUserName(newRegion, newRegionName);
      double rgba[4];
      meshRegion->GetColor(rgba);
      newRegion->SetColor(rgba[0], rgba[1], rgba[2], rgba[3]);
      newRegion->SetPointInside(entRegion->GetPointInside());
    }
    else
    {
      vtkWarningMacro("There is no face in this solid region ??")
    }
  }
  iterSolReg->Delete();
  vtkModelMaterial* meshRegMaterial = meshRegion->GetMaterial();
  meshModel->DestroyModelGeometricEntity(meshRegion);
  meshModel->DestroyMaterial(meshRegMaterial);
  return true;
}

void vtkCMBIncorporateMeshOperator::AddSolidFaceCells(vtkDiscreteModel* meshModel,
  vtkDiscreteModelFace* faceEntity, vtkIdList* newCellIds,
  std::map<vtkIdType, vtkIdType>& SolidToMeshPointMap)
{
  typedef std::map<vtkIdType, vtkIdType>::const_iterator c_iter;
  vtkPolyData* facePoly = vtkPolyData::SafeDownCast(faceEntity->GetGeometry());
  const DiscreteMesh& meshMaster = meshModel->GetMesh();

  vtkIdType numCells = facePoly->GetNumberOfCells();
  vtkIdType npts, *pts;
  vtkNew<vtkIdList> newPtsList;
  for (vtkIdType i = 0; i < numCells; i++)
  {
    newPtsList->Initialize();
    facePoly->GetCellPoints(i, npts, pts);

    DiscreteMesh::Face f(facePoly->GetCellType(i));
    for (vtkIdType j = 0; j < npts; j++)
    {
      c_iter foundPointId = SolidToMeshPointMap.find(pts[j]);
      if (foundPointId != SolidToMeshPointMap.end())
      {
        f.AddExistingPointId(foundPointId->second);
      }
      else
      {
        f.AddNewPoint(facePoly->GetPoint(pts[j]));
      }
    }
    DiscreteMesh::FaceResult face = meshMaster.AddFace(f);
    newCellIds->InsertNextId(face.CellId);
    for (vtkIdType j = 0; j < npts; j++)
    {
      SolidToMeshPointMap[pts[j]] = face[j];
    }
  }
  meshModel->UpdateMesh();
}

vtkIdType vtkCMBIncorporateMeshOperator::FindPointsCell(const DiscreteMesh* targetMesh,
  vtkIdType ptId, vtkPolyData* soucePoly, vtkIdList* points, vtkIdList* visitedCellMask,
  std::map<vtkIdType, vtkIdType>& SolidToMeshPointMap)
{
  vtkIdType cellId = -1, currentCellId;
  vtkNew<vtkIdList> tgtPpts;
  vtkNew<vtkIdList> pCells;

  targetMesh->GetCellsUsingPoint(ptId, pCells.GetPointer(), DiscreteMesh::FACE_DATA);
  const vtkIdType numCells = pCells->GetNumberOfIds();
  if (numCells == 0)
  {
    return cellId;
  }

  for (vtkIdType i = 0; i < numCells; i++)
  {
    currentCellId = pCells->GetId(i);
    if (visitedCellMask->IsId(currentCellId) >= 0)
    {
      continue;
    }
    targetMesh->GetCellPointIds(currentCellId, tgtPpts.GetPointer());
    if (this->IsSameCellPoints(
          targetMesh, tgtPpts.GetPointer(), soucePoly, points, SolidToMeshPointMap))
    {
      return currentCellId;
    }
  }
  return cellId;
}

bool vtkCMBIncorporateMeshOperator::IsSameCellPoints(const DiscreteMesh* targetMesh,
  vtkIdList* tpts, vtkPolyData* soucePoly, vtkIdList* spts,
  std::map<vtkIdType, vtkIdType>& SolidToMeshPointMap)
{
  const vtkIdType tnpts = tpts->GetNumberOfIds();
  const vtkIdType snpts = spts->GetNumberOfIds();
  if (tnpts != snpts)
  {
    return false;
  }

  double sp[3], tp[3];
  vtkIdType tpid, spid;
  int numFound = 0;
  bool foundone = true;
  for (vtkIdType si = 0; si < snpts && foundone; si++)
  {
    spid = spts->GetId(si);
    soucePoly->GetPoint(spid, sp);
    foundone = false;
    for (vtkIdType ti = 0; ti < tnpts; ti++)
    {
      tpid = tpts->GetId(ti);
      targetMesh->GetPoint(tpid, tp);
      // We need some tolerance here, since the points
      // may be changed during transform calculation.
      if (fabs(tp[0] - sp[0]) < 1e-3 && fabs(tp[1] - sp[1]) < 1e-3 && fabs(tp[2] - sp[2]) < 1e-3)
      {
        foundone = true;
        numFound++;
        SolidToMeshPointMap[tpid] = spid;
        break;
      }
    }
  }
  return numFound == snpts;
}

void vtkCMBIncorporateMeshOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Number of Solid Meshes: " << this->SolidMeshes.size() << endl;
}
