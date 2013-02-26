#ifndef DISCRETEMESHCLASSIFICATION_H
#define DISCRETEMESHCLASSIFICATION_H

#include "vtkType.h"
#include <vector> //needed for vector

template<class EntityType>
class DiscreteMeshClassification
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
EntityType* DiscreteMeshClassification<EntityType>::GetEntity(
                                                          vtkIdType id) const
{
  return this->Classification[id].second;
}

//=============================================================================
template<class EntityType>
vtkIdType DiscreteMeshClassification<EntityType>::GetEntityIndex(
                                                          vtkIdType id) const
{
  return this->Classification[id].first;
}

//=============================================================================
template<class EntityType>
void DiscreteMeshClassification<EntityType>::SetEntity(vtkIdType id,
                                                       vtkIdType entityIndex,
                                                       EntityType* entity)
{
  this->Classification[id] =  classificationStorageType(entityIndex,entity);
}


//=============================================================================
template<class EntityType>
void DiscreteMeshClassification<EntityType>::resize(vtkIdType size)
{
  classificationStorageType empty(-1,NULL);
  this->Classification.resize(size, empty);
}

#endif // DISCRETEMESHCLASSIFICATION_H
