//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_bridge_exodus_Bridge_h
#define __smtk_bridge_exodus_Bridge_h

#include "smtk/model/Cursor.h"
#include "smtk/model/Bridge.h"

#include "smtk/common/UUID.h"
#include "smtk/common/UUIDGenerator.h"

#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

#include <map>
#include <vector>

namespace smtk {
  namespace bridge {
    namespace exodus {

// ++ 2 ++
/// The types of entities in an Exodus "model"
enum EntityType
{
  EXO_MODEL,        //!< An entire Exodus dataset (file)
  EXO_BLOCK,        //!< Exodus blocks are groups of cells inside a dataset.
  EXO_SIDE_SET,     //!< Exodus side sets are groups of cell boundaries.
  EXO_NODE_SET,     //!< Exodus node sets are groups of points.

  EXO_INVALID       //!< The handle is invalid
};

/// A "handle" for a VTK entity (point, cell, property, etc.)
struct EntityHandle
{
  int modelNumber;         //!< An offset in the vector of models (m_models).
  EntityType entityType;   //!< Describes the type of entity.
  vtkIdType entityId;      //!< The offset in the array of a model's blocks describing this entity.

  EntityHandle()
    : modelNumber(-1), entityType(EXO_INVALID), entityId(-1)
    { }

  EntityHandle(EntityType etyp, int emod, vtkIdType eid)
    : modelNumber(emod), entityType(etyp), entityId(eid)
    { }

  bool isValid() const
    { return this->entityType != EXO_INVALID && this->modelNumber >= 0; }

  EntityHandle parent() const
    {
    return
      (this->entityType == EXO_MODEL || this->entityType == EXO_INVALID) ?
      EntityHandle() :
      EntityHandle(EXO_MODEL, this->modelNumber, -1);
    }
};
// -- 2 --

// ++ 1 ++
/**\brief Implement a bridge from VTK unstructured grids to SMTK.
  */
class Bridge : public smtk::model::Bridge
{
public:
  typedef std::vector<vtkSmartPointer<vtkMultiBlockDataSet> > ModelVector_t;
  typedef std::map<smtk::model::Cursor,EntityHandle> ReverseIdMap_t;

  smtkTypeMacro(Bridge);
  smtkCreateMacro(smtk::model::Bridge);
  smtkSharedFromThisMacro(smtk::model::Bridge);
  smtkDeclareModelingKernel();
  typedef smtk::model::BridgedInfoBits BridgedInfoBits;
  virtual ~Bridge();
  virtual BridgedInfoBits allSupportedInformation() const
    { return smtk::model::BRIDGE_EVERYTHING; }

  EntityHandle toEntity(const smtk::model::Cursor& eid);
  smtk::model::Cursor toCursor(const EntityHandle& ent);

  static vtkInformationStringKey* SMTK_UUID_KEY();

  smtk::model::ModelEntity addModel(vtkSmartPointer<vtkMultiBlockDataSet>& model);

protected:
  friend class Operator;

  Bridge();

  virtual BridgedInfoBits transcribeInternal(
    const smtk::model::Cursor& entity,
    BridgedInfoBits requestedInfo);

  template<typename T>
  T* toBlock(const EntityHandle& handle);

  std::string toBlockName(const EntityHandle& handle) const;

  smtk::common::UUIDGenerator m_uuidGen;
  ModelVector_t m_models;
  ReverseIdMap_t m_revIdMap;
  // std::map<EntityHandle,smtk::model::Cursor> m_fwdIdMap; // not needed; store UUID in vtkInformation.
  // -- 1 --

  bool addTessellation(
    const smtk::model::Cursor&,
    const EntityHandle&);

private:
  Bridge(const Bridge&); // Not implemented.
  void operator = (const Bridge&); // Not implemented.
};

template<typename T>
T* Bridge::toBlock(const EntityHandle& handle)
{
  if (
    handle.entityType == EXO_INVALID ||
    (handle.entityType != EXO_MODEL && handle.entityId < 0) ||
    handle.modelNumber < 0 ||
    handle.modelNumber > static_cast<int>(this->m_models.size()))
    return NULL;

  int blockId = -1; // Where in the VTK dataset is the entity type data?
  switch (handle.entityType)
    {
  case EXO_MODEL:
    return dynamic_cast<T*>(
      this->m_models[handle.modelNumber].GetPointer());
    break;
  case EXO_BLOCK:    blockId = 0; break;
  case EXO_SIDE_SET: blockId = 4; break;
  case EXO_NODE_SET: blockId = 7; break;
  default:
    return NULL;
    }
  vtkMultiBlockDataSet* typeSet =
    vtkMultiBlockDataSet::SafeDownCast(
      this->m_models[handle.modelNumber]->GetBlock(blockId));
  if (!typeSet || typeSet->GetNumberOfBlocks() >= handle.entityId)
    return NULL;
  return dynamic_cast<T*>(typeSet->GetBlock(handle.entityId));
}


    } // namespace exodus
  } // namespace bridge
} // namespace smtk


#endif // __smtk_bridge_exodus_Bridge_h
