//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkGenerateSimpleModelOperator.h"

#include "DiscreteMesh.h"
#include "ModelParserHelper.h"
#include "vtkCellData.h"
#include "vtkCleanPolyData.h"
#include "vtkCreateModelEdgesOperator.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkIdList.h"
#include "vtkModelMaterial.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkTriangleFilter.h"

#include <map>

vtkStandardNewMacro(vtkGenerateSimpleModelOperator);

vtkGenerateSimpleModelOperator::vtkGenerateSimpleModelOperator()
{
  this->OperateSucceeded = 0;
}

vtkGenerateSimpleModelOperator::~vtkGenerateSimpleModelOperator()
{
}

void vtkGenerateSimpleModelOperator::Operate(vtkDiscreteModelWrapper* modelWrapper,
                                             vtkAlgorithm* inputFilter, int cleanInput)
{
  this->OperateSucceeded = 0;
  if(!modelWrapper)
    {
    vtkErrorMacro("Passed in a null model wrapper.");
    return;
    }

  if(!inputFilter)
    {
    vtkErrorMacro("Input is empty.");
    return;
    }
  inputFilter->Update();
  vtkSmartPointer<vtkPolyData> poly = vtkPolyData::SafeDownCast(inputFilter->GetOutputDataObject(0));
  if(!poly)
    {
    vtkErrorMacro("Input does not provide a polydata.");
    return;
    }

  // Clean up the poly data to make sure we have all triangles
  // and proper connectivity
  if(cleanInput)
    {
    vtkNew<vtkTriangleFilter> triangulate;
    triangulate->SetInputDataObject(poly);
    vtkNew<vtkCleanPolyData> cleanPolyData;
    cleanPolyData->SetInputConnection(triangulate->GetOutputPort());
    cleanPolyData->Update();
    poly = vtkPolyData::SafeDownCast(cleanPolyData->GetOutput());
    }

  // See if we have model face IDs associated with the cells;
  // if so, then create a model face for each unique value
  // instead of a single global model face.
  vtkIdTypeArray* modelFaceIds =
    vtkIdTypeArray::SafeDownCast(
      poly->GetCellData()->GetArray(
        ModelParserHelper::GetModelFaceTagName()));

  // Now see if the input polydata had id-type pedigree IDs.
  // If so, create a new model entity group for each unique ID;
  // we'll add model faces to the groups as we go.
  // This assumes that no items with the same model-face IDs
  // have different pedigree IDs (because model faces are not
  // decomposable).
  // TODO: Handle non-vtkIdType pedigree IDs.
  vtkIdTypeArray* pedigreeIds = vtkIdTypeArray::SafeDownCast(
    poly->GetCellData()->GetPedigreeIds());
  vtkIdType* pedigree = pedigreeIds && pedigreeIds->GetNumberOfTuples() > 0 ?
    pedigreeIds->GetPointer(0) : NULL;

  // Create the associated model using \a poly as the mesh:
  DiscreteMesh mesh(poly);
  vtkDiscreteModel* model = modelWrapper->GetModel();
  model->Reset();
  model->SetMesh(mesh);

  int zero = 0;
  vtkModelMaterial* material = NULL;
  if (!modelFaceIds)
    { // Add all triangles to one model face
    material = pedigree ? model->BuildMaterial(pedigree[0]) : model->BuildMaterial();
    vtkDiscreteModelFace* face = vtkDiscreteModelFace::SafeDownCast(
      model->BuildModelFace(0, NULL, &zero, material));
    vtkNew<vtkIdList> cellIds;
    cellIds->SetNumberOfIds(poly->GetNumberOfCells());
    for(vtkIdType i=0;i<poly->GetNumberOfCells();i++)
      {
      cellIds->SetId(i, i);
      }
    face->AddCellsToGeometry(cellIds.GetPointer());
    if (pedigree)
      { // Assume that our sole model face has the same pedigree ID for every cell:
      vtkDiscreteModelEntity* modelEnt = face;
      /*vtkDiscreteModelEntityGroup* grp =*/
        model->BuildModelEntityGroup(
          vtkDiscreteModelEntityGroupType, 1, &modelEnt, pedigree[0]);
      }
    }
  else
    { // Add triangles to specified model faces.
    std::map<vtkIdType,vtkSmartPointer<vtkIdList> > decomposition;
    std::map<vtkIdType,vtkSmartPointer<vtkIdList> >::iterator fit;
    std::map<vtkIdType,vtkModelMaterial*> groups;
    std::map<vtkIdType,vtkModelMaterial*>::iterator git;
    vtkIdType* modelFaces = modelFaceIds->GetPointer(0);
    for (vtkIdType i = 0; i < poly->GetNumberOfCells(); ++i)
      {
      fit = decomposition.find(modelFaces[i]);
      if (fit == decomposition.end())
        {
        fit = decomposition.insert(
          std::pair<vtkIdType,vtkSmartPointer<vtkIdList> >(
            modelFaces[i], vtkSmartPointer<vtkIdList>::New())).first;
        }
      fit->second->InsertNextId(i);

      if (pedigree)
        {
        vtkIdType pedId = pedigree[i];
        if (groups.find(pedId) == groups.end())
          {
          material = model->BuildMaterial(pedId);
          groups[pedId] = material;
          cout << "Add face " << fit->first << " to group " << pedId << "\n";
          }
        }
      }
    if (!pedigree)
      { // Create one default material.
      material = model->BuildMaterial();
      groups[material->GetUniquePersistentId()] = material;
      }
    for (fit = decomposition.begin(); fit != decomposition.end(); ++fit)
      {
      vtkDiscreteModelFace* face = vtkDiscreteModelFace::SafeDownCast(
        model->BuildModelFace(0, NULL, &zero,
          pedigree ? groups[pedigree[fit->second->GetId(0)]] : material));
      face->AddCellsToGeometry(fit->second.GetPointer());
      }
    }

  vtkNew<vtkCreateModelEdgesOperator> createEdges;
  createEdges->ShowEdgesOn();
  createEdges->Operate(modelWrapper);
  this->OperateSucceeded = createEdges->GetOperateSucceeded();
  if(this->OperateSucceeded)
    {
    modelWrapper->InitializeWithModelGeometry();
    }
}

void vtkGenerateSimpleModelOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
