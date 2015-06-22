//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Model_h
#define __smtk_model_Model_h

#include "smtk/model/EntityRef.h"

namespace smtk {
  namespace model {

class CellEntity;
class Group;
class Model;
typedef std::vector<CellEntity> CellEntities;
typedef std::vector<Model> Models;

/**\brief A entityref subclass that provides methods specific to models.
  *
  */
class SMTKCORE_EXPORT Model : public EntityRef
{
public:
  SMTK_ENTITYREF_CLASS(Model,EntityRef,isModel);

  ModelGeometryStyle geometryStyle() const;

  void setEmbeddingDimension(int dim);

  EntityRef parent() const;
  SessionRef session() const;
  void setSession(const SessionRef& sess);

  CellEntities cells() const;
  Groups groups() const;
  Models submodels() const;

  Model& addCell(const CellEntity& c);
  Model& removeCell(const CellEntity& c);
  template<typename T> Model& addCells(const T& container);
  template<typename T> Model& removeCells(const T& container);

  Model& addGroup(const Group& g);
  Model& removeGroup(const Group& g);
  template<typename T> Model& addGroups(const T& container);
  template<typename T> Model& removeGroups(const T& container);

  Model& addSubmodel(const Model& m);
  Model& removeSubmodel(const Model& m);
  template<typename T> Model& addSubmodels(const T& container);
  template<typename T> Model& removeSubmodels(const T& container);

  OperatorPtr op(const std::string& operatorName) const;
  StringList operatorNames() const;

  void assignDefaultNames();
};

/// Add all the free cells in \a container to this model.
template<typename T> Model& Model::addCells(const T& container)
{
  this->embedEntities(container);
  return *this;
}

/// Add all the free cells in \a container to this model.
template<typename T> Model& Model::removeCells(const T& container)
{
  this->unembedEntities(container);
  return *this;
}

/// Add all the groups in \a container to this model.
template<typename T> Model& Model::addGroups(const T& container)
{
  for (typename T::const_iterator it = container.begin(); it != container.end(); ++it)
    {
    this->addGroup(*it);
    }
  return *this;
}

/// Add all the groups in \a container to this model.
template<typename T> Model& Model::removeGroups(const T& container)
{
  for (typename T::const_iterator it = container.begin(); it != container.end(); ++it)
    {
    this->removeGroup(*it);
    }
  return *this;
}

/// Add all the models in \a container as submodels to this model.
template<typename T> Model& Model::addSubmodels(const T& container)
{
  for (typename T::const_iterator it = container.begin(); it != container.end(); ++it)
    {
    this->addSubmodel(*it);
    }
  return *this;
}

/// Add all the models in \a container as submodels to this model.
template<typename T> Model& Model::removeSubmodels(const T& container)
{
  for (typename T::const_iterator it = container.begin(); it != container.end(); ++it)
    {
    this->removeSubmodel(*it);
    }
  return *this;
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Model_h
