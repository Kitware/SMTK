//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_CellEntity_h
#define __smtk_model_CellEntity_h

#include "smtk/model/EntityRef.h"
#include "smtk/model/UseEntity.h" // For UseEntities
#include "smtk/model/ShellEntity.h" // For ShellEntities
#include "smtk/model/EntityRefArrangementOps.h" // For appendAllRelations

namespace smtk {
  namespace model {

class CellEntity;
class Model;
typedef std::vector<CellEntity> CellEntities;

/**\brief A entityref subclass with methods specific to cell entities.
  *
  */
class SMTKCORE_EXPORT CellEntity : public EntityRef
{
public:
  SMTK_ENTITYREF_CLASS(CellEntity,EntityRef,isCellEntity);

  Model model() const;
  CellEntities boundingCells() const;
  UseEntities boundingCellUses(Orientation orientation) const;

  template<typename T> T boundingCellsAs() const;
  template<typename T> T inclusions() const;
  template<typename T> T uses() const;

  ShellEntity findShellEntityContainingUse(const UseEntity& bdyUse);
  ShellEntities findShellEntitiesContainingCell(const CellEntity& cell);
};

template<typename T>
T CellEntity::boundingCellsAs() const
{
  CellEntities tmp = this->boundingCells();
  T result(tmp.begin(), tmp.end());
  return result;
}

template<typename T>
T CellEntity::inclusions() const
{
  T result;
  EntityRefArrangementOps::appendAllRelations(*this, INCLUDES, result);
  return result;
}

template<typename T>
T CellEntity::uses() const
{
  T result;
  EntityRefArrangementOps::appendAllRelations(*this, HAS_USE, result);
  return result;
}

  } // namespace model
} // namespace smtk

#endif // __smtk_model_CellEntity_h
