/*=========================================================================

Copyright (c) 2013 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/

#include "PythonExportGridInfo2D.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkModelEdge.h"
#include "vtkModelItemIterator.h"
#include "vtkModelGridRepresentation.h"
#include "vtkModelGeneratedGridRepresentation.h"

#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkSmartPointer.h>

using namespace smtk::model;


PythonExportGridInfo2D::PythonExportGridInfo2D(vtkDiscreteModel *model)
  : PythonExportGridInfo(model)
{
}


PythonExportGridInfo2D::~PythonExportGridInfo2D()
{
}


std::vector<int>
PythonExportGridInfo2D::
analysisGridCells(int modelEntityId, GridInfo::ApiStatus& status)
{
  std::vector<int> cellIds;  // return value

  vtkModelGeneratedGridRepresentation *gridRep =
    this->getGeneratedGridRep(status);
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


std::vector<std::pair<int, int> >
PythonExportGridInfo2D::
boundaryItemsOf(int modelEntityId, GridInfo::ApiStatus& status)
{
  std::vector<std::pair<int, int> > gridItems;  // return value

  vtkModelGeneratedGridRepresentation *gridRep =
    this->getGeneratedGridRep(status);
  if (!gridRep)
    {
    return gridItems;
    }

  // Not sure if this is the right file for this code, but here goes:
  vtkSmartPointer<vtkIdList> analysisCellSides = vtkSmartPointer<vtkIdList>::New();
  vtkSmartPointer<vtkIdTypeArray> analysisCellEdges = vtkSmartPointer<vtkIdTypeArray>::New();

  vtkDiscreteModelEntityGroup* modelEntityGroup =
    vtkDiscreteModelEntityGroup::SafeDownCast(
      m_model->GetModelEntity(vtkDiscreteModelEntityGroupType, modelEntityId));
  if (modelEntityGroup == NULL)
    {
    status.returnType = GridInfo::NOT_AVAILABLE;
    status.errorMessage = "model entity not found";
    return gridItems;
    }

  vtkSmartPointer<vtkModelItemIterator> edges;
  edges.TakeReference(modelEntityGroup->NewModelEntityIterator());
  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(edges->GetCurrentItem());
    std::cout << "edge " << edge->GetUniquePersistentId() << std::endl;
    if(gridRep->GetModelEdgeAnalysisPoints(m_model, edge->GetUniquePersistentId(), analysisCellEdges))
      {
      vtkIdType values[2];
      for(vtkIdType j=0;j<analysisCellEdges->GetNumberOfTuples();j++)
        {
        analysisCellEdges->GetTupleValue(j, values);
        std::cout << "edge->GetNumberOfAdjacentModelFaces()" << edge->GetNumberOfAdjacentModelFaces() << std::endl;
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


std::vector<std::pair<int, int> >
PythonExportGridInfo2D::
asBoundaryItems(int modelEntityId, int boundedModelEntityId,
                GridInfo::ApiStatus& status)
{
  std::vector<std::pair<int, int> > gridItems;  // return value

  // Current code does not support boundedModelEntityId
  if (boundedModelEntityId >= 0)
    {
    status.returnType = GridInfo::NOT_AVAILABLE;
    status.errorMessage = "Current version does not support boundedModelEntityId != -1";
    return gridItems;
    }

  vtkDiscreteModelEntityGroup* modelEntityGroup =
    vtkDiscreteModelEntityGroup::SafeDownCast(
      m_model->GetModelEntity(vtkDiscreteModelEntityGroupType, modelEntityId));
  if (modelEntityGroup == NULL)
    {
    status.returnType = GridInfo::NOT_AVAILABLE;
    status.errorMessage = "model entity not found";
    return gridItems;
    }

  vtkModelGridRepresentation *gridRep = m_model->GetAnalysisGridInfo();

  vtkIdList *cellIds = vtkIdList::New();
  vtkIdList *cellSides = vtkIdList::New();
  if (gridRep->GetBoundaryGroupAnalysisFacets(this->m_model,
    modelEntityId, cellIds, cellSides) == false)
    {
    status.returnType = GridInfo::NOT_AVAILABLE;
    status.errorMessage = "Could not get group facets";
    return gridItems;
    }
  //std::cout << "Number of facets: " << cellIds->GetNumberOfIds() << std::endl;


  // Copy cellIds & cellSides to gridItems
  for (vtkIdType i=0; i<cellIds->GetNumberOfIds(); ++i)
    {
    gridItems.push_back(std::pair<int, int>(cellIds->GetId(i), cellSides->GetId(i)));
    }

  cellIds->Delete();
  cellSides->Delete();

  status.returnType = GridInfo::OK;
  status.errorMessage = "";
  return gridItems;
}

std::vector<std::pair<int, int> >
PythonExportGridInfo2D::
edgeGridItems(int boundaryGroupId, ApiStatus& status)
{
  std::vector<std::pair<int, int> > gridItems;  // return value

  vtkDiscreteModelEntityGroup* modelEntityGroup =
    vtkDiscreteModelEntityGroup::SafeDownCast(
      m_model->GetModelEntity(vtkDiscreteModelEntityGroupType, boundaryGroupId));
  if (modelEntityGroup == NULL)
    {
    status.returnType = GridInfo::NOT_AVAILABLE;
    status.errorMessage = "Boundary group not found";
    return gridItems;
    }

  vtkModelGridRepresentation *gridRep = m_model->GetAnalysisGridInfo();

  vtkSmartPointer<vtkIdTypeArray> edgePoints = vtkSmartPointer<vtkIdTypeArray>::New();
  vtkSmartPointer<vtkModelItemIterator> edges;
  edges.TakeReference(modelEntityGroup->NewModelEntityIterator());
  for(edges->Begin();!edges->IsAtEnd();edges->Next())
    {
    vtkModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(edges->GetCurrentItem());
    //std::cout << "edge " << edge->GetUniquePersistentId() << std::endl;
    if(gridRep->GetModelEdgeAnalysisPoints(m_model, edge->GetUniquePersistentId(), edgePoints))
      {
      vtkIdType pointIds[2];
      for(vtkIdType i=0; i<edgePoints->GetNumberOfTuples(); i++)
        {
        edgePoints->GetTupleValue(i, pointIds);
        gridItems.push_back(std::pair<int, int>(pointIds[0], pointIds[1]));
        }
      }
    else
      {
      std::cout << "NO ANALYSIS POINTS! Edge id " << edge->GetUniquePersistentId()  << std::endl;
      // return;
      }
    }

  status.returnType = GridInfo::OK;
  status.errorMessage = "";
  return gridItems;
}


std::string
PythonExportGridInfo2D::
getClassName() const
{
  return "PythonExportGridInfo2D";
}
