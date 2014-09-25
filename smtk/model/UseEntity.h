//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_UseEntity_h
#define __smtk_model_UseEntity_h

#include "smtk/model/Cursor.h"
#include "smtk/model/CursorArrangementOps.h" // For shellEntities<T>().

namespace smtk {
  namespace model {

class CellEntity;
class ShellEntity;
class UseEntity;
typedef std::vector<UseEntity> UseEntities;

/**\brief A cursor subclass that provides methods specific to entity-use records.
  *
  * An entity-use record provides a way to reference how a cell is
  * employed (**used**) to bound a higher-dimensional cell.
  * Entity-use records reference a particular **sense** in which their
  * corresponding cell is used (e.g., an edge may used in its forward or
  * backward sense). Entity-use records are associated with a particular
  * higher-dimensional half-space that their corresponding cell defines
  * (or helps define).
  *
  * An edge's forward or backward senses may each be used by any number
  * of different face loops.
  *
  * A face has 0, 1, or 2 use records, depending on whether it appears
  * in the shells of 0, 1, or 2 volumes.
  *
  * A vertex may have any number of use records; one should exist for each
  * volume, free face, or free edge attached to the vertex.
  *
  * The boundingShellEntity() is the ShellEntity in which this
  * use participates as part or all of a higher-dimensional use's boundary.
  * VertexUse entities may participate in multiple chains, and so
  * boundingShellEntities() is provided.
  * VolumeUse entities should always return an invalid boundingShellEntity()
  * (until the need for space-time or higher-dimensional modeling drives
  * a change).
  */
class SMTKCORE_EXPORT UseEntity : public Cursor
{
public:
  SMTK_CURSOR_CLASS(UseEntity,Cursor,isUseEntity);

  CellEntity cell() const;
  ShellEntity boundingShellEntity() const;
  template<typename T> T boundingShellEntities() const;
  template<typename T> T shellEntities() const;
  Orientation orientation() const;
  int sense() const;

  UseEntity& setBoundingShellEntity(const ShellEntity& shell);
  UseEntity& addShellEntity(const ShellEntity& shell);
  template<typename T> UseEntity& addShellEntities(const T& shellContainer);
};

template<typename T> T UseEntity::boundingShellEntities() const
{
  T container;
  CursorArrangementOps::appendAllRelations(*this, HAS_SHELL, container);
  return container;
}

template<typename T> T UseEntity::shellEntities() const
{
  T container;
  CursorArrangementOps::appendAllRelations(*this, INCLUDES, container);
  return container;
}

template<typename T>
UseEntity& UseEntity::addShellEntities(const T& shellContainer)
{
  for (typename T::const_iterator it = shellContainer.begin(); it != shellContainer.end(); ++it)
    {
    this->addShellEntity(*it);
    }
  return *this;
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_UseEntity_h
