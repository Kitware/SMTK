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


//=============================================================================
ModelVertexClassification::ModelVertexClassification(vtkDiscreteModel* model)
{
  this->Model = model;
  this->ModelVertInfo = std::map<vtkIdType,ModelVertexInfo>();
  //iterate the model creating a set of ids
  vtkModelItemIterator* vertices = model->NewIterator(vtkModelVertexType);
  for(vertices->Begin();!vertices->IsAtEnd();vertices->Next())
    {
    vtkDiscreteModelVertex* vertex =
      vtkDiscreteModelVertex::SafeDownCast(vertices->GetCurrentItem());

    const vtkIdType pointId = vertex->GetPointId();
    const vtkIdType uniqueModelId = vertex->GetUniquePersistentId();
    this->ModelVertInfo[pointId] = ModelVertexInfo(uniqueModelId,vertex);
    }
}

//=============================================================================
vtkDiscreteModelVertex* ModelVertexClassification::GetModelVertex(
                                                      vtkIdType pointId )
{
  typedef std::map<vtkIdType,ModelVertexInfo>::iterator iterator;
  iterator i = this->ModelVertInfo.find(pointId);
  vtkDiscreteModelVertex* info = NULL;
  if(i != this->ModelVertInfo.end())
    {
    //iterator is key:value, we need the values second item
    info = i->second.second;
    }
  return info;
}

//=============================================================================
vtkIdType ModelVertexClassification::GetModelId( vtkIdType pointId )
{
  typedef std::map<vtkIdType,ModelVertexInfo>::const_iterator iterator;
  iterator i = this->ModelVertInfo.find(pointId);
  vtkIdType info = -1;
  if(i != this->ModelVertInfo.end())
    {
    //iterator is key:value, we need the values first item
    info = i->second.first;
    }
  return info;
}

  //=============================================================================
bool ModelVertexClassification::HasModelVertex( vtkIdType pointId ) const
{
  return this->ModelVertInfo.count(pointId) == 1;
}

//=============================================================================
std::pair<vtkIdType, vtkDiscreteModelVertex*>
  ModelVertexClassification::AddModelVertex( vtkIdType pointId,
  bool bCreateGeometry )
{
  typedef std::map<vtkIdType,ModelVertexInfo>::iterator iterator;
  typedef std::pair<vtkIdType,ModelVertexInfo> insertPair;

  vtkDiscreteModelVertex* emptyVertex = NULL;
  ModelVertexInfo empty(-1,emptyVertex);
  insertPair insertedItem(pointId,empty);

  std::pair<iterator,bool> inserted =this->ModelVertInfo.insert(insertedItem);
  if(inserted.second)
    {
    //we have inserted the item, so that means we have to actually
    //update the model.
    vtkModelVertex* newVertex = this->Model->BuildModelVertex(
      pointId, bCreateGeometry);
    vtkDiscreteModelVertex* dvert =
                        vtkDiscreteModelVertex::SafeDownCast(newVertex);
    const vtkIdType vertexModelId = newVertex->GetUniquePersistentId();

    //assign into the map the new value for the key
    inserted.first->second = ModelVertexInfo(vertexModelId,dvert);
    }

  //inserted fist is the iterator, and we need to return the value
  //of the iterator which is the second item
  return inserted.first->second;
}
