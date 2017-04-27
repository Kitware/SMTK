//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "PythonExportGridInfo3D.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkModelGeneratedGridRepresentation.h"
#include "vtkModelGridRepresentation.h"

#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkSmartPointer.h>

#include <sstream>

using namespace smtk::model;

PythonExportGridInfo3D::PythonExportGridInfo3D(vtkDiscreteModel* model)
  : PythonExportGridInfo(model)
{
}

PythonExportGridInfo3D::~PythonExportGridInfo3D()
{
}

std::vector<int> PythonExportGridInfo3D::analysisGridCells(
  int modelEntityId, GridInfo::ApiStatus& status)
{
  std::vector<int> cellIds; // return value

  vtkModelGeneratedGridRepresentation* gridRep = this->getGeneratedGridRep(status);
  if (!gridRep)
  {
    return cellIds;
  }

  if (gridRep->GetGroupFacetIds(this->m_model, modelEntityId, cellIds) == false)
  {
    status.returnType = GridInfo::NOT_AVAILABLE;
    status.errorMessage = "Could not get group facet ids";
    return cellIds;
  }

  status.returnType = GridInfo::OK;
  status.errorMessage = "";
  return cellIds;
}

std::vector<std::pair<int, int> > PythonExportGridInfo3D::boundaryItemsOf(
  int modelEntityId, GridInfo::ApiStatus& status)
{
  std::vector<std::pair<int, int> > gridItems; // return value

  vtkModelEntity* modelEnt = m_model->GetModelEntity(modelEntityId);
  if (modelEnt == NULL)
  {
    status.returnType = GridInfo::ENTITY_NOT_FOUND;
    std::stringstream ss;
    ss << "model entity " << modelEntityId << " not found";
    status.errorMessage = ss.str();
    return gridItems;
  }

  vtkModelGridRepresentation* gridRep = this->m_model->GetAnalysisGridInfo();
  vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();
  vtkSmartPointer<vtkIdList> cellSides = vtkSmartPointer<vtkIdList>::New();
  if (gridRep->GetBoundaryGroupAnalysisFacets(m_model, modelEntityId, cellIds, cellSides))
  {
    // Copy cellIds & cellSides to gridItems
    for (vtkIdType i = 0; i < cellIds->GetNumberOfIds(); ++i)
    {
      gridItems.push_back(std::pair<int, int>(cellIds->GetId(i), cellSides->GetId(i)));
    }
  }
  else
  {
    status.returnType = GridInfo::NOT_AVAILABLE;
    std::stringstream ss;
    ss << "no facets found for model entity " << modelEntityId;
    status.errorMessage = ss.str();
    return gridItems;
  }

  status.returnType = GridInfo::OK;
  status.errorMessage = "";
  return gridItems;
}

std::vector<std::pair<int, int> > PythonExportGridInfo3D::asBoundaryItems(
  int modelEntityId, int boundedModelEntityId, GridInfo::ApiStatus& status)
{
  std::vector<std::pair<int, int> > gridItems; // return value

  // Current code does not support boundedModelEntityId
  if (boundedModelEntityId >= 0)
  {
    status.returnType = GridInfo::NOT_AVAILABLE;
    status.errorMessage = "Current version does not support boundedModelEntityId != -1";
    return gridItems;
  }

  vtkDiscreteModelEntityGroup* modelEntityGroup = vtkDiscreteModelEntityGroup::SafeDownCast(
    m_model->GetModelEntity(vtkDiscreteModelEntityGroupType, modelEntityId));
  if (modelEntityGroup == NULL)
  {
    status.returnType = GridInfo::NOT_AVAILABLE;
    status.errorMessage = "model entity not found";
    return gridItems;
  }

  vtkModelGridRepresentation* gridRep = m_model->GetAnalysisGridInfo();

  vtkIdList* cellIds = vtkIdList::New();
  vtkIdList* cellSides = vtkIdList::New();
  if (gridRep->GetBoundaryGroupAnalysisFacets(this->m_model, modelEntityId, cellIds, cellSides) ==
    false)
  {
    status.returnType = GridInfo::NOT_AVAILABLE;
    status.errorMessage = "Could not get group facets";
    return gridItems;
  }
  //std::cout << "Number of facets: " << cellIds->GetNumberOfIds() << std::endl;

  // Copy cellIds & cellSides to gridItems
  for (vtkIdType i = 0; i < cellIds->GetNumberOfIds(); ++i)
  {
    gridItems.push_back(std::pair<int, int>(cellIds->GetId(i), cellSides->GetId(i)));
  }

  cellIds->Delete();
  cellSides->Delete();

  status.returnType = GridInfo::OK;
  status.errorMessage = "";
  return gridItems;
}

std::string PythonExportGridInfo3D::getClassName() const
{
  return "PythonExportGridInfo3D";
}
