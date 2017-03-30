//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "ModelVertexClassification.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkModelItemIterator.h"

#include <map> //need to store the set of point ids

struct ModelVertexClassification::Internals
{
  typedef std::pair<vtkIdType, vtkDiscreteModelVertex*> ModelVertexInfo;
  std::map<vtkIdType, ModelVertexInfo> ModelVertInfo;
  vtkDiscreteModel* Model;
};

ModelVertexClassification::ModelVertexClassification(vtkDiscreteModel* model)
{
  this->Internal = new Internals();

  this->Internal->Model = model;
  this->Internal->ModelVertInfo = std::map<vtkIdType, Internals::ModelVertexInfo>();
  //iterate the model creating a set of ids
  vtkModelItemIterator* vertices = model->NewIterator(vtkModelVertexType);
  for (vertices->Begin(); !vertices->IsAtEnd(); vertices->Next())
  {
    vtkDiscreteModelVertex* vertex =
      vtkDiscreteModelVertex::SafeDownCast(vertices->GetCurrentItem());

    const vtkIdType pointId = vertex->GetPointId();
    const vtkIdType uniqueModelId = vertex->GetUniquePersistentId();
    this->Internal->ModelVertInfo[pointId] = Internals::ModelVertexInfo(uniqueModelId, vertex);
  }
}

ModelVertexClassification::~ModelVertexClassification()
{
  delete this->Internal;
}

vtkDiscreteModelVertex* ModelVertexClassification::GetModelVertex(vtkIdType pointId)
{
  typedef std::map<vtkIdType, Internals::ModelVertexInfo>::iterator iterator;
  iterator i = this->Internal->ModelVertInfo.find(pointId);
  vtkDiscreteModelVertex* info = NULL;
  if (i != this->Internal->ModelVertInfo.end())
  {
    //iterator is key:value, we need the values second item
    info = i->second.second;
  }
  return info;
}

vtkIdType ModelVertexClassification::GetModelId(vtkIdType pointId)
{
  typedef std::map<vtkIdType, Internals::ModelVertexInfo>::const_iterator iterator;
  iterator i = this->Internal->ModelVertInfo.find(pointId);
  vtkIdType info = -1;
  if (i != this->Internal->ModelVertInfo.end())
  {
    //iterator is key:value, we need the values first item
    info = i->second.first;
  }
  return info;
}

bool ModelVertexClassification::HasModelVertex(vtkIdType pointId) const
{
  return this->Internal->ModelVertInfo.count(pointId) == 1;
}

vtkDiscreteModelVertex* ModelVertexClassification::AddModelVertex(
  vtkIdType pointId, bool bCreateGeometry)
{
  vtkIdType modelId;
  return this->AddModelVertex(pointId, bCreateGeometry, modelId);
}

vtkDiscreteModelVertex* ModelVertexClassification::AddModelVertex(
  vtkIdType pointId, bool bCreateGeometry, vtkIdType& modelId)
{
  typedef std::map<vtkIdType, Internals::ModelVertexInfo>::iterator iterator;
  typedef std::pair<vtkIdType, Internals::ModelVertexInfo> insertPair;

  vtkDiscreteModelVertex* emptyVertex = NULL;
  Internals::ModelVertexInfo empty(-1, emptyVertex);
  insertPair insertedItem(pointId, empty);

  std::pair<iterator, bool> inserted = this->Internal->ModelVertInfo.insert(insertedItem);
  if (inserted.second)
  {
    //we have inserted the item, so that means we have to actually
    //update the model.
    vtkModelVertex* newVertex = this->Internal->Model->BuildModelVertex(pointId, bCreateGeometry);
    vtkDiscreteModelVertex* dvert = vtkDiscreteModelVertex::SafeDownCast(newVertex);
    const vtkIdType vertexModelId = newVertex->GetUniquePersistentId();

    //assign into the map the new value for the key
    inserted.first->second = Internals::ModelVertexInfo(vertexModelId, dvert);
  }

  //inserted fist is the iterator, and we need to return the value
  //of the iterator which is the second item
  // return inserted.first->second;
  modelId = inserted.first->second.first;
  return inserted.first->second.second;
}
