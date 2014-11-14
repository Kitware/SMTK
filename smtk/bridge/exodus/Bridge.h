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

#include <map>

class vtkUnstructuredGrid;

namespace smtk {
  namespace bridge {
    namespace exodus {

// ++ 2 ++
/// The types of entities in an Exodus "model"
enum EntityType
{
  EXO_BLOCK,        // Exodus blocks are groups of cells
  EXO_SIDE_SET,     // Exodus side sets are groups of cell boundaries
  EXO_NODE_SET      // Exodus node sets are groups of points
};

/// A "handle" for a VTK entity (point, cell, property, etc.)
struct EntityHandle {
  EntityType entityType;   //!< Describes the type of entity
  int entityRelation;      //!< A modifier for the entity type (to select an array or boundary)
  vtkIdType entityId;      //!< The offset in the array of entities describing this entity.
};
// -- 2 --

// ++ 1 ++
/**\brief Implement a bridge from VTK unstructured grids to SMTK.
  */
class Bridge : public smtk::model::Bridge
{
public:
  smtkDeclareModelingKernel();
  typedef smtk::shared_ptr<Bridge> Ptr;
  typedef smtk::model::BridgedInfoBits BridgedInfoBits;
  static BridgePtr create();
  virtual ~Bridge();
  virtual BridgedInfoBits allSupportedInformation() const
    { return BRIDGE_EVERYTHING; }

  EntityHandle toEntity(const smtk::model::Cursor& eid);
  smtk::model::Cursor toCursor(const EntityHandle& ent);

  static vtkInformationStringKey* SMTK_UUID_KEY();

protected:
  Bridge();

  virtual BridgedInfoBits transcribeInternal(
    const smtk::model::Cursor& entity,
    BridgedInfoBits requestedInfo);

  template<typename T>
  T* toBlock(const EntityHandle& handle);

  smtk::model::ManagerPtr m_manager;
  smtk::common::UUIDGenerator m_uuidGen;
  vtkNew<vtkMultiBlockDataSet> m_model;
  // std::map<EntityHandle,smtk::model::Cursor> m_fwdIdMap; // not needed; store UUID in field data.
  typedef std::map<smtk::model::Cursor,EntityHandle> ReverseIdMap_t;
  ReverseIdMap_t m_revIdMap;
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
  if (ent.entityId < 0)
    return NULL;

  int blockId = -1; // Where in the VTK dataset is the entity type data?
  switch (ent.entityType)
    {
  case EXO_BLOCK:    blockId = 0; break;
  case EXO_SIDE_SET: blockId = 4; break;
  case EXO_NODE_SET: blockId = 7; break;
  default:
    return NULL;
    }
  vtkMultiBlockDataSet* typeSet =
    vtkMultiBlockDataSet::SafeDownCast(
      this->m_model->GetBlock(blockId));
  if (!typeSet || typeSet->GetNumberOfBlocks() >= ent.entityId)
    return NULL;
  return dynamic_cast<T*>(typeSet->GetBlock(ent.entityId));
}


    } // namespace exodus
  } // namespace bridge
} // namespace smtk


#endif // __smtk_bridge_exodus_Bridge_h
