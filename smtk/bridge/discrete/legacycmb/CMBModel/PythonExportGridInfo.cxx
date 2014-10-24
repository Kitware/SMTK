//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "PythonExportGridInfo.h"
#include "vtkDiscreteModel.h"
#include "vtkModelGridRepresentation.h"
#include "vtkModelGeneratedGridRepresentation.h"

#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkSmartPointer.h>
#include <sstream>

using namespace smtk::model;


PythonExportGridInfo::PythonExportGridInfo(vtkDiscreteModel *model)
  : m_model(model)
{
}


PythonExportGridInfo::~PythonExportGridInfo()
{
}


int
PythonExportGridInfo::
dimension(GridInfo::ApiStatus& status) const
{
  // Return model dimension
  if (NULL == m_model)
    {
    status.returnType = GridInfo::NOT_AVAILABLE;
    status.errorMessage = "model is null";
    return -1;
    }

  int dim = m_model->GetModelDimension();
  status.returnType = GridInfo::OK;
  status.errorMessage = "";
  return dim;
}

int
PythonExportGridInfo::
cellType(int /* gridCellId */, GridInfo::ApiStatus& status)
{
  int gridCellType = -1;  // return value
  status.returnType = GridInfo::NOT_AVAILABLE;
  status.errorMessage = this->getClassName() + "::cellType() not implemented";
  return gridCellType;
}

std::vector<int>
PythonExportGridInfo::
pointIds(int modelEntityId, PointClosure closure, GridInfo::ApiStatus& status)
{
  std::vector<int> pointIdList;  // return value

  // Check that model entity exists
  vtkModelEntity *modelEnt = m_model->GetModelEntity(modelEntityId);
  if (modelEnt == NULL)
    {
    status.returnType = GridInfo::ENTITY_NOT_FOUND;
    std::stringstream ss;
    ss << "Did not find model entity for id " << modelEntityId;
    status.errorMessage = ss.str();
    return pointIdList;
    }

  // TODO: Should we check that modelEnt is a model edge here?

  // Don't need macro to cast to "generated" mesh
  vtkModelGridRepresentation *gridRep = m_model->GetAnalysisGridInfo();

  // Map closure argument to grid bcsType
  // Found by inspection of vtkCMBMeshGridRepresentationServer
  vtkIdType bcsType = 0;
  switch (closure)
    {
    case GridInfo::ALL_POINTS:       bcsType = 1;  break;
    case GridInfo::BOUNDARY_POINTS:  bcsType = 2;  break;
    case GridInfo::INTERIOR_POINTS:  bcsType = 3;  break;
    }

  // Get point ids from grid, returned as vtkIdList
  vtkIdList *vtkPointIdList = vtkIdList::New();
  if (gridRep->GetBCSNodalAnalysisGridPointIds(m_model,
    modelEntityId, bcsType, vtkPointIdList) == false)
    {
    status.returnType = GridInfo::NOT_AVAILABLE;
    status.errorMessage = "";
    return pointIdList;
    }

  // Copy ids from vtkIdList to std::vector
  for (vtkIdType i=0; i<vtkPointIdList->GetNumberOfIds(); ++i)
    {
    pointIdList.push_back(vtkPointIdList->GetId(i));
    }

  vtkPointIdList->Delete();
  //std::cout << "Number of points: " << pointIds.size() << std::endl;

  status.returnType = GridInfo::OK;
  status.errorMessage = "";
  return pointIdList;
}


std::vector<int>
PythonExportGridInfo::
cellPointIds(int gridCellId, GridInfo::ApiStatus& status)
{
  std::vector<int> pointIdList;  // return value
  vtkModelGeneratedGridRepresentation *gridRep =
    this->getGeneratedGridRep(status);
  if (!gridRep)
    {
    status.returnType = GridInfo::NOT_AVAILABLE;
    status.errorMessage = "GridInfo doesn't support this operation";
    return pointIdList;
    }

  if (gridRep->GetCellPointIds(gridCellId, pointIdList) == false)
    {
    status.returnType = GridInfo::NOT_AVAILABLE;
    status.errorMessage = "Could not get cell point ids";
    return pointIdList;
    }

  status.returnType = GridInfo::OK;
  status.errorMessage = "";
  return pointIdList;
}


std::vector<double>
PythonExportGridInfo::
pointLocation(int gridPointId, GridInfo::ApiStatus& status)
{
  std::vector<double> coords;  // return type
  vtkModelGeneratedGridRepresentation *gridRep =
    this->getGeneratedGridRep(status);
  if (!gridRep)
    {
    return coords;
    }

  if (gridRep->GetPointLocation(gridPointId, coords) == false)
    {
    status.returnType = GridInfo::NOT_AVAILABLE;
    status.errorMessage = "Could not get point location";
    return coords;
    }

  status.returnType = GridInfo::OK;
  status.errorMessage = "";
  return coords;
}


std::string
PythonExportGridInfo::
nodeElemSetClassification(int /* modelEntityId */, GridInfo::ApiStatus& status)
{
  std::string identifier;  // return value
  status.returnType = GridInfo::NOT_AVAILABLE;
  status.errorMessage = this->getClassName() +
    "::nodeElemSetClassification() not implemented";
  return identifier;
}


std::string
PythonExportGridInfo::
sideSetClassification(int /* modelEntityId */, GridInfo::ApiStatus& status)
{
  std::string identifier;  // return value
  status.returnType = GridInfo::NOT_AVAILABLE;
  status.errorMessage = this->getClassName() +
    "::sideSetClassification() not implemented";
  return identifier;
}


std::vector<std::pair<int, int> >
PythonExportGridInfo::
edgeGridItems(int boundaryGroupId, ApiStatus& status)
{
  std::vector<std::pair<int, int> > items;  // return value
  status.returnType = GridInfo::NOT_AVAILABLE;
  status.errorMessage = this->getClassName() +
    "::edgeGridItems() not implemneted";
  return items;
}


vtkModelGeneratedGridRepresentation*
PythonExportGridInfo::
getGeneratedGridRep(ApiStatus& status)
{
  if (NULL == this->m_model)
    {
    status.returnType = GridInfo::NOT_AVAILABLE;
    status.errorMessage = "model is null";
    return NULL;
    }
  vtkModelGeneratedGridRepresentation *gridRep =
    vtkModelGeneratedGridRepresentation::
    SafeDownCast(this->m_model->GetAnalysisGridInfo());

  return gridRep;
}
