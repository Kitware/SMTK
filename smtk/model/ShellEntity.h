//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_ShellEntity_h
#define __smtk_model_ShellEntity_h

#include "smtk/model/EntityRef.h"
#include "smtk/model/EntityRefArrangementOps.h"

namespace smtk {
  namespace model {

class CellEntity;
class ShellEntity;
typedef std::vector<ShellEntity> ShellEntities;
class UseEntity;
typedef std::vector<UseEntity> UseEntities;

/**\brief A entityref subclass with methods specific to shell entities.
  *
  * A shell is a collection of oriented cell-uses that form a
  * subset of the boundary of a higher-dimensional parent cell.
  * A shell may contain other shells.
  */
class SMTKCORE_EXPORT ShellEntity : public EntityRef
{
public:
  SMTK_ENTITYREF_CLASS(ShellEntity,EntityRef,isShellEntity);

  CellEntity boundingCell() const;
  UseEntity boundingUseEntity() const;

  bool contains(const UseEntity& bdyUse) const;

  template<typename T> T uses() const;
  template<typename T> T cellsOfUses() const;

  ShellEntity containingShellEntity() const;
  template<typename T> T containedShellEntities() const;

  ShellEntity& addUse(const UseEntity& use);
  template<typename T> ShellEntity& addUses(const T& useContainer);
};

/**\brief Return the uses (cells with an orientation, or sense) composing this shell.
  *
  * When the shell is 1-dimensional, these uses will be ordered curve segments
  * that match head-to-tail and describe a closed loop.
  *
  * TODO: Decide how to handle multiple closed loops: should an "empty" shell (that
  * returns uses().empty()) be created with multiple containedShells()? Or should
  * shells be allowed to have siblings?
  */
template<typename T>
T ShellEntity::uses() const
{
  T result;
  EntityRefArrangementOps::appendAllRelations(*this, HAS_USE, result);
  return result;
}

/**\brief Return all the shell-entities contained inside this one.
  *
  */
template<typename T>
T ShellEntity::containedShellEntities() const
{
  T result;
  EntityRefArrangementOps::appendAllRelations(*this, INCLUDES, result);
  return result;
}

template<typename T>
ShellEntity& ShellEntity::addUses(const T& useContainer)
{
  for (typename T::const_iterator it = useContainer.begin(); it != useContainer.end(); ++it)
    {
    this->addUse(*it);
    }
  return *this;
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ShellEntity_h
