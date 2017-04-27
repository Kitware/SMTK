//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "PythonExportGridInfo2D.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkModelEdge.h"
#include "vtkModelGeneratedGridRepresentation.h"
#include "vtkModelGridRepresentation.h"
#include "vtkModelItemIterator.h"

#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkSmartPointer.h>

using namespace smtk::model;

PythonExportGridInfo2D::PythonExportGridInfo2D(vtkDiscreteModel* model)
  : PythonExportGridInfo(model)
{
}

PythonExportGridInfo2D::~PythonExportGridInfo2D()
{
}

std::vector<int> PythonExportGridInfo2D::analysisGridCells(
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

std::vector<std::pair<int, int> > PythonExportGridInfo2D::boundaryItemsOf(
  int modelEntityId, GridInfo::ApiStatus& status)
{
  std::vector<std::pair<int, int> > gridItems; // return value

  vtkModelGeneratedGridRepresentation* gridRep = this->getGeneratedGridRep(status);
  if (!gridRep)
  {
    return gridItems;
  }

  // Not sure if this is the right file for this code, but here goes:
  vtkSmartPointer<vtkIdList> analysisCellSides = vtkSmartPointer<vtkIdList>::New();
  vtkSmartPointer<vtkIdTypeArray> analysisCellEdges = vtkSmartPointer<vtkIdTypeArray>::New();

  vtkDiscreteModelEntityGroup* modelEntityGroup = vtkDiscreteModelEntityGroup::SafeDownCast(
    m_model->GetModelEntity(vtkDiscreteModelEntityGroupType, modelEntityId));
  if (modelEntityGroup == NULL)
  {
    status.returnType = GridInfo::NOT_AVAILABLE;
    status.errorMessage = "model entity not found";
    return gridItems;
  }

  vtkSmartPointer<vtkModelItemIterator> edges;
  edges.TakeReference(modelEntityGroup->NewModelEntityIterator());
  for (edges->Begin(); !edges->IsAtEnd(); edges->Next())
  {
    vtkModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(edges->GetCurrentItem());
    std::cout << "edge " << edge->GetUniquePersistentId() << std::endl;
    if (gridRep->GetModelEdgeAnalysisPoints(
          m_model, edge->GetUniquePersistentId(), analysisCellEdges))
    {
      vtkIdType values[2];
      for (vtkIdType j = 0; j < analysisCellEdges->GetNumberOfTuples(); j++)
      {
        analysisCellEdges->GetTypedTuple(j, values);
        std::cout << "edge->GetNumberOfAdjacentModelFaces()"
                  << edge->GetNumberOfAdjacentModelFaces() << std::endl;
        // if(edge->GetNumberOfAdjacentModelFaces() >= 2)
        //   {
        //       file << "MDS " << values[0]+1 << " " << values[1]+1 << " " << bcIndex << endl;
        //       }
        //     else
        //       {
        //       file << "EGS " << values[0]+1 << " " << values[1]+1 << " " << bcIndex << endl;
        //       }
        //   }
      }
    }
    else
    {
      std::cout << "NO ANALYSIS POINTS!" << std::endl;
      // return;
    }
  }

  status.returnType = GridInfo::OK;
  status.errorMessage = "";
  return gridItems;
}

std::vector<std::pair<int, int> > PythonExportGridInfo2D::asBoundaryItems(
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

std::vector<std::pair<int, int> > PythonExportGridInfo2D::edgeGridItems(
  int boundaryGroupId, ApiStatus& status)
{
  std::vector<std::pair<int, int> > gridItems; // return value

  vtkDiscreteModelEntityGroup* modelEntityGroup = vtkDiscreteModelEntityGroup::SafeDownCast(
    m_model->GetModelEntity(vtkDiscreteModelEntityGroupType, boundaryGroupId));
  if (modelEntityGroup == NULL)
  {
    status.returnType = GridInfo::NOT_AVAILABLE;
    status.errorMessage = "Boundary group not found";
    return gridItems;
  }

  vtkModelGridRepresentation* gridRep = m_model->GetAnalysisGridInfo();

  vtkSmartPointer<vtkIdTypeArray> edgePoints = vtkSmartPointer<vtkIdTypeArray>::New();
  vtkSmartPointer<vtkModelItemIterator> edges;
  edges.TakeReference(modelEntityGroup->NewModelEntityIterator());
  for (edges->Begin(); !edges->IsAtEnd(); edges->Next())
  {
    vtkModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(edges->GetCurrentItem());
    //std::cout << "edge " << edge->GetUniquePersistentId() << std::endl;
    if (gridRep->GetModelEdgeAnalysisPoints(m_model, edge->GetUniquePersistentId(), edgePoints))
    {
      vtkIdType pointIds[2];
      for (vtkIdType i = 0; i < edgePoints->GetNumberOfTuples(); i++)
      {
        edgePoints->GetTypedTuple(i, pointIds);
        gridItems.push_back(std::pair<int, int>(pointIds[0], pointIds[1]));
      }
    }
    else
    {
      std::cout << "NO ANALYSIS POINTS! Edge id " << edge->GetUniquePersistentId() << std::endl;
      // return;
    }
  }

  status.returnType = GridInfo::OK;
  status.errorMessage = "";
  return gridItems;
}

std::string PythonExportGridInfo2D::getClassName() const
{
  return "PythonExportGridInfo2D";
}
