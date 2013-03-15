#ifndef __MESHCLASSIFICATION_H
#define __MESHCLASSIFICATION_H

#include "vtkType.h"
#include <vector> //needed for vector

template<class EntityType>
class MeshClassification
{
public:
  // Description:
  // Functions to get the object that the passed in ID refers too
  EntityType* GetEntity(vtkIdType meshId) const;
  vtkIdType GetEntityIndex(vtkIdType meshId) const;

  void SetEntity(vtkIdType meshId, vtkIdType entityIndex, EntityType* entity);

  void resize(vtkIdType size);

private:
  typedef std::pair<vtkIdType,EntityType* > classificationStorageType;
  classificationStorageType GetElement(vtkIdType meshIndex ) const;
  void SetElement(vtkIdType meshIndex, classificationStorageType& element);

  //zero index are faces, 1 index is edges
  std::vector<classificationStorageType> Classifications[2];
};

//=============================================================================
template<class EntityType>
EntityType* MeshClassification<EntityType>::GetEntity(vtkIdType meshId) const
{
  return this->GetElement(meshId).second;
}

//=============================================================================
template<class EntityType>
vtkIdType MeshClassification<EntityType>::GetEntityIndex(vtkIdType meshId) const
{
  return this->GetElement(meshId).first;
}

//=============================================================================
template<class EntityType>
void MeshClassification<EntityType>::SetEntity(vtkIdType meshId,
                                               vtkIdType entityIndex,
                                               EntityType* entity)
{
  classificationStorageType element(entityIndex,entity);
  this->SetElement(meshId,element);
}


//=============================================================================
template<class EntityType>
void MeshClassification<EntityType>::resize(vtkIdType size)
{
  classificationStorageType empty(-1,NULL);
  this->Classifications[0].resize(size, empty);
  this->Classifications[1].resize(size, empty);
}

//=============================================================================
template<class EntityType>
typename MeshClassification<EntityType>::classificationStorageType
MeshClassification<EntityType>::GetElement( vtkIdType meshIndex ) const
{
  const int is_edge = static_cast<int>(meshIndex < 0);
  meshIndex = meshIndex ^ -is_edge;
  return this->Classifications[is_edge][meshIndex];
}

//=============================================================================
template<class EntityType>
void MeshClassification<EntityType>::SetElement(vtkIdType meshIndex,
                                            classificationStorageType& element)
{
  const int is_edge = static_cast<int>(meshIndex < 0);
  meshIndex = meshIndex ^ -is_edge;
  this->Classifications[is_edge][meshIndex]=element;
}

#endif // DISCRETEMESHCLASSIFICATION_H
