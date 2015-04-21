//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_exodus_Session_h
#define __smtk_session_exodus_Session_h

#include "smtk/bridge/exodus/Exports.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Session.h"

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
  EXO_MODEL,        //!< An entire Exodus dataset (a file).
  EXO_BLOCK,        //!< Exodus BLOCKs are groups of cells inside a MODEL.
  EXO_SIDE_SET,     //!< Exodus SIDE_SETs are groups of cell boundaries in a MODEL.
  EXO_NODE_SET,     //!< Exodus NODE_SETs are groups of points in a MODEL.

  EXO_INVALID       //!< The handle is invalid
};

/// A "handle" for an Exodus entity (file, block, side set, or node set)
struct SMTKEXODUSSESSION_EXPORT EntityHandle
{
  EntityType entityType;   //!< Describes the type of entity.
  int modelNumber;         //!< An offset in the vector of models (m_models).
  vtkIdType entityId;      //!< The offset in the array of a model's blocks describing this entity.

  /// Construct an invalid handle.
  EntityHandle()
    : entityType(EXO_INVALID), modelNumber(-1), entityId(-1)
    { }

  /// Construct a possibly-valid handle.
  EntityHandle(EntityType etyp, int emod, vtkIdType eid)
    : entityType(etyp), modelNumber(emod), entityId(eid)
    { }

  bool isValid() const
    { return this->entityType != EXO_INVALID && this->modelNumber >= 0; }

  /// Given a handle, return its parent if it has one.
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
/**\brief Implement a session from Exodus mesh files to SMTK.
  *
  * This session uses the VTK Exodus reader to obtain
  * information from Exodus files, with each element block,
  * side set, and node set represented as a vtkUnstructuredGrid.
  */
class SMTKEXODUSSESSION_EXPORT Session : public smtk::model::Session
{
public:
  typedef std::vector<vtkSmartPointer<vtkMultiBlockDataSet> > ModelVector_t;
  typedef std::map<smtk::model::EntityRef,EntityHandle> ReverseIdMap_t;

  smtkTypeMacro(Session);
  smtkSuperclassMacro(smtk::model::Session);
  smtkSharedFromThisMacro(smtk::model::Session);
  smtkCreateMacro(smtk::model::Session);
  smtkDeclareModelingKernel();
  typedef smtk::model::SessionInfoBits SessionInfoBits;
  virtual ~Session();
  virtual SessionInfoBits allSupportedInformation() const
    { return smtk::model::SESSION_EVERYTHING; }

  EntityHandle toEntity(const smtk::model::EntityRef& eid);
  smtk::model::EntityRef toEntityRef(const EntityHandle& ent);

  std::vector<EntityHandle> childrenOf(const EntityHandle& ent);

  static vtkInformationStringKey* SMTK_UUID_KEY();

  smtk::model::Model addModel(vtkSmartPointer<vtkMultiBlockDataSet>& model);

protected:
  friend class Operator;

  Session();

  virtual SessionInfoBits transcribeInternal(
    const smtk::model::EntityRef& entity,
    SessionInfoBits requestedInfo);

  template<typename T>
  T* toBlock(const EntityHandle& handle);

  std::string toBlockName(const EntityHandle& handle) const;

  smtk::common::UUIDGenerator m_uuidGen;
  ModelVector_t m_models;
  ReverseIdMap_t m_revIdMap;
  // std::map<EntityHandle,smtk::model::EntityRef> m_fwdIdMap; // not needed; store UUID in vtkInformation.
  // -- 1 --

  bool addTessellation(
    const smtk::model::EntityRef&,
    const EntityHandle&);

private:
  Session(const Session&); // Not implemented.
  void operator = (const Session&); // Not implemented.
};

// ++ 3 ++
template<typename T>
T* Session::toBlock(const EntityHandle& handle)
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
  if (!typeSet || handle.entityId >= typeSet->GetNumberOfBlocks())
    return NULL;
  return dynamic_cast<T*>(typeSet->GetBlock(handle.entityId));
}
// -- 3 --


    } // namespace exodus
  } // namespace bridge
} // namespace smtk


#endif // __smtk_session_exodus_Session_h
