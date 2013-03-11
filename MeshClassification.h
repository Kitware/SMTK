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
  EntityType* GetEntity(vtkIdType id) const;
  vtkIdType GetEntityIndex(vtkIdType id) const;

  void SetEntity(vtkIdType id, vtkIdType entityIndex, EntityType* entity);

  void resize(vtkIdType size);

private:
  typedef std::pair<vtkIdType,EntityType* > classificationStorageType;
  std::vector<classificationStorageType> Classification;
};

//=============================================================================
template<class EntityType>
EntityType* MeshClassification<EntityType>::GetEntity(
                                                          vtkIdType id) const
{
  return this->Classification[id].second;
}

//=============================================================================
template<class EntityType>
vtkIdType MeshClassification<EntityType>::GetEntityIndex(
                                                          vtkIdType id) const
{
  return this->Classification[id].first;
}

//=============================================================================
template<class EntityType>
void MeshClassification<EntityType>::SetEntity(vtkIdType id,
                                                       vtkIdType entityIndex,
                                                       EntityType* entity)
{
  this->Classification[id] =  classificationStorageType(entityIndex,entity);
}


//=============================================================================
template<class EntityType>
void MeshClassification<EntityType>::resize(vtkIdType size)
{
  classificationStorageType empty(-1,NULL);
  this->Classification.resize(size, empty);
}

#endif // DISCRETEMESHCLASSIFICATION_H
