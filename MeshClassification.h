#ifndef __MESHCLASSIFICATION_H
#define __MESHCLASSIFICATION_H

#include "vtkType.h"
#include <vector> //needed for vector

template<class EntityType>
class MeshClassification
{
public:
  enum DataType{ FACE_DATA=0, EDGE_DATA=1, BOTH_DATA=2};

  // Description:
  // Functions to get the object that the passed in ID refers too
  EntityType* GetEntity(vtkIdType meshId) const;
  vtkIdType GetEntityIndex(vtkIdType meshId) const;

  //Unchecked setting of a mesh id entity. If meshId is not a valid
  //id expect undefined behavior
  void SetEntity(vtkIdType meshId, vtkIdType entityIndex, EntityType* entity);

  //Will verify that meshId is within current classification memory, otherwise
  //will resize memory to include it
  void AddEntity(vtkIdType meshId, vtkIdType entityIndex, EntityType* entity);

  vtkIdType size(DataType type) const;
  void resize(vtkIdType size,DataType type);

  //returns true if the mesh id has a valid Entity assigned to it
  bool IsValidMeshId(vtkIdType meshId) const;

private:
  typedef std::pair<vtkIdType,EntityType* > classificationStorageType;
  typedef std::vector<classificationStorageType> classificationVectorType;

  inline vtkIdType ToIndex(vtkIdType meshId) const
    { //inline for performance
    return meshId ^ -(static_cast<int>(meshId < 0));
    }
  inline classificationVectorType& GetVectorFromId(vtkIdType id)
    { return this->Classifications[static_cast<int>(id < 0)]; }

  inline const classificationVectorType& GetVectorFromId(vtkIdType id) const
    { return this->Classifications[static_cast<int>(id < 0)]; }

  //zero index are faces, 1 index is edges
  classificationVectorType Classifications[2];
};

//=============================================================================
template<class EntityType>
EntityType* MeshClassification<EntityType>::GetEntity(vtkIdType meshId) const
{
  const vtkIdType index = this->ToIndex(meshId);
  return this->GetVectorFromId(meshId)[index].second;
}

//=============================================================================
template<class EntityType>
vtkIdType MeshClassification<EntityType>::GetEntityIndex(vtkIdType meshId) const
{
  const vtkIdType index = this->ToIndex(meshId);
  return this->GetVectorFromId(meshId)[index].first;
}

//=============================================================================
template<class EntityType>
void MeshClassification<EntityType>::SetEntity(vtkIdType meshId,
                                               vtkIdType entityIndex,
                                               EntityType* entity)
{
  classificationStorageType element(entityIndex,entity);
  const vtkIdType index = this->ToIndex(meshId);
  this->GetVectorFromId(meshId)[index]=element;
}

//=============================================================================
template<class EntityType>
void MeshClassification<EntityType>::AddEntity(vtkIdType meshId,
                                               vtkIdType entityIndex,
                                               EntityType* entity)
{
  classificationStorageType element(entityIndex,entity);
  std::vector<classificationStorageType>& v = this->GetVectorFromId(meshId);
  const vtkIdType index = this->ToIndex(meshId);

  const std::size_t sizeOfStorage = v.size();
  if(sizeOfStorage <= index)
    {
    v.resize(index+1); //resize to make meshIndex a valid index
    }
  v[index]=element;
}

//=============================================================================
template<class EntityType>
void MeshClassification<EntityType>::resize(vtkIdType size, DataType type)
{
  //abuse enums are ints to make edge (1) be negative
  this->GetVectorFromId(-type).resize(size);
}

//=============================================================================
template<class EntityType>
vtkIdType MeshClassification<EntityType>::size(DataType type) const
{
  //abuse enums are ints to make edge (1) be negative
  return this->GetVectorFromId(-type).size();
}


//=============================================================================
template<class EntityType>
bool MeshClassification<EntityType>::IsValidMeshId(vtkIdType meshId) const
{
  return (index < this->GetVectorFromId(meshId).size()) &&
         (this->GetElement(meshId).second!=NULL);
}

#endif // DISCRETEMESHCLASSIFICATION_H
