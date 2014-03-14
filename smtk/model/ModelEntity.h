#ifndef __smtk_model_ModelEntity_h
#define __smtk_model_ModelEntity_h

#include "smtk/model/Cursor.h"

namespace smtk {
  namespace model {

class CellEntity;
class GroupEntity;
class ModelEntity;
typedef std::vector<CellEntity> CellEntities;
typedef std::vector<GroupEntity> GroupEntities;
typedef std::vector<ModelEntity> ModelEntities;

/**\brief A cursor subclass that provides methods specific to models.
  *
  */
class SMTKCORE_EXPORT ModelEntity : public Cursor
{
public:
  SMTK_CURSOR_CLASS(ModelEntity,Cursor,isModelEntity);

  Cursor parent() const;

  CellEntities cells() const;
  GroupEntities groups() const;
  ModelEntities submodels() const;

  ModelEntity& addCell(const CellEntity& c);
  ModelEntity& removeCell(const CellEntity& c);
  template<typename T> ModelEntity& addCells(const T& container);
  template<typename T> ModelEntity& removeCells(const T& container);

  ModelEntity& addGroup(const GroupEntity& g);
  ModelEntity& removeGroup(const GroupEntity& g);
  template<typename T> ModelEntity& addGroups(const T& container);
  template<typename T> ModelEntity& removeGroups(const T& container);

  ModelEntity& addSubmodel(const ModelEntity& m);
  ModelEntity& removeSubmodel(const ModelEntity& m);
  template<typename T> ModelEntity& addSubmodels(const T& container);
  template<typename T> ModelEntity& removeSubmodels(const T& container);

  OperatorPtr op(const std::string& operatorName) const;
  Operators operators() const;

  BridgeBasePtr bridge() const;
};

/// Add all the free cells in \a container to this model.
template<typename T> ModelEntity& ModelEntity::addCells(const T& container)
{
  this->embedEntities(container);
  return *this;
}

/// Add all the free cells in \a container to this model.
template<typename T> ModelEntity& ModelEntity::removeCells(const T& container)
{
  this->unembedEntities(container);
  return *this;
}

/// Add all the groups in \a container to this model.
template<typename T> ModelEntity& ModelEntity::addGroups(const T& container)
{
  for (typename T::const_iterator it = container.begin(); it != container.end(); ++it)
    {
    this->addGroup(*it);
    }
  return *this;
}

/// Add all the groups in \a container to this model.
template<typename T> ModelEntity& ModelEntity::removeGroups(const T& container)
{
  for (typename T::const_iterator it = container.begin(); it != container.end(); ++it)
    {
    this->removeGroup(*it);
    }
  return *this;
}

/// Add all the models in \a container as submodels to this model.
template<typename T> ModelEntity& ModelEntity::addSubmodels(const T& container)
{
  for (typename T::const_iterator it = container.begin(); it != container.end(); ++it)
    {
    this->addSubmodel(*it);
    }
  return *this;
}

/// Add all the models in \a container as submodels to this model.
template<typename T> ModelEntity& ModelEntity::removeSubmodels(const T& container)
{
  for (typename T::const_iterator it = container.begin(); it != container.end(); ++it)
    {
    this->removeSubmodel(*it);
    }
  return *this;
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ModelEntity_h
