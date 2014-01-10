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
  template<typename T> ModelEntity& addCells(const T& container);

  ModelEntity& addGroup(const GroupEntity& g);
  template<typename T> ModelEntity& addGroups(const T& container);

  ModelEntity& addSubmodel(const ModelEntity& m);
  template<typename T> ModelEntity& addSubmodels(const T& container);
};

/// Add all the free cells in \a container to this model.
template<typename T> ModelEntity& ModelEntity::addCells(const T& container)
{
  this->embedEntities(container);
  return *this;
}

/// Add all the groups in \a container to this model.
template<typename T> ModelEntity& ModelEntity::addGroups(const T& container)
{
  this->embedEntities(container);
  return *this;
}

/// Add all the models in \a container as submodels to this model.
template<typename T> ModelEntity& ModelEntity::addSubmodels(const T& container)
{
  this->embedEntities(container);
  return *this;
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ModelEntity_h
