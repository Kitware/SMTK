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
#include "vtkDataObjectTreeIterator.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

#include <map>
#include <vector>

class vtkInformationStringKey;
class vtkInformationIntegerKey;

namespace smtk {
  namespace bridge {
    namespace exodus {

class Session;

// ++ 2 ++
/// The types of entities in an Exodus "model"
enum EntityType
{
  EXO_MODEL      = 0x01,   //!< An entire Exodus dataset (a file).
  EXO_BLOCK      = 0x02,   //!< Exodus BLOCKs are groups of cells inside a MODEL.
  EXO_SIDE_SET   = 0x03,   //!< Exodus SIDE_SETs are groups of cell boundaries in a MODEL.
  EXO_NODE_SET   = 0x04,   //!< Exodus NODE_SETs are groups of points in a MODEL.

  EXO_BLOCKS     = 0x05,   //!< A group of Exodus BLOCKs.
  EXO_SIDE_SETS  = 0x06,   //!< A group of Exodus SIDE_SETs.
  EXO_NODE_SETS  = 0x07,   //!< A group of Exodus NODE_SETs.

  EXO_INVALID    = 0xff    //!< The handle is invalid
};

SMTKEXODUSSESSION_EXPORT std::string EntityTypeNameString(EntityType etype);

/// A "handle" for an Exodus entity (file, block, side set, or node set)
struct SMTKEXODUSSESSION_EXPORT EntityHandle
{
  int m_modelNumber; //!< An offset in the vector of models (m_models) owned by the session, whose model owns m_object.
  vtkSmartPointer<vtkDataObject> m_object; //!< The dataset being presented as this entity.
  Session* m_session; //!< The session owning this entity.

  EntityHandle();
  EntityHandle(int emod, vtkDataObject* obj, Session* sess);
  EntityHandle(int emod, vtkDataObject* obj, vtkMultiBlockDataSet* parent, int idxInParent, Session* sess);

  bool isValid() const;

  EntityType entityType() const;
  std::string name() const;
  int pedigree() const;
  bool visible() const;

  int modelNumber() const { return this->m_modelNumber; }

  EntityHandle parent() const;

  template<typename T>
  T* object() const;

  template<typename T>
  T childrenAs(int depth) const
    {
    T container;
    this->appendChildrenTo(container, depth);
    return container;
    }

  template<typename T>
  void appendChildrenTo(T& container, int depth) const;
};
// -- 2 --

typedef std::vector<EntityHandle> EntityHandleArray;

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
  typedef std::pair<vtkMultiBlockDataSet*, int> ParentAndIndex_t;
  typedef std::map<vtkDataObject*, ParentAndIndex_t> ChildParentMap_t;

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

  // VTK keys used to mark blocks.
  static vtkInformationIntegerKey* SMTK_DIMENSION();
  static vtkInformationIntegerKey* SMTK_VISIBILITY();
  static vtkInformationIntegerKey* SMTK_GROUP_TYPE();
  static vtkInformationIntegerKey* SMTK_PEDIGREE();
  static vtkInformationStringKey* SMTK_UUID_KEY();

  smtk::model::Model addModel(vtkSmartPointer<vtkMultiBlockDataSet>& model);

protected:
  friend class Operator;
  friend struct EntityHandle;

  Session();

  virtual SessionInfoBits transcribeInternal(
    const smtk::model::EntityRef& entity,
    SessionInfoBits requestedInfo,
    int depth = -1);

  smtk::common::UUIDGenerator m_uuidGen;
  ModelVector_t m_models;
  ReverseIdMap_t m_revIdMap;
  ChildParentMap_t m_cpMap; // vtkMultiBlockDataSet doesn't provide a fast way to obtain parent of leaf datasets.
  // std::map<EntityHandle,smtk::model::EntityRef> m_fwdIdMap; // not needed; store UUID in vtkInformation.
  // -- 1 --

  bool addTessellation(
    const smtk::model::EntityRef&,
    const EntityHandle&);

  size_t numberOfModels() const;
  vtkMultiBlockDataSet* modelOfHandle(const EntityHandle& h) const;
  vtkMultiBlockDataSet* parent(vtkDataObject* obj) const;
  int parentIndex(vtkDataObject* obj) const;
  bool ensureChildParentMapEntry(vtkDataObject* child, vtkMultiBlockDataSet* parent, int idxInParent);

  virtual smtk::model::SessionIOPtr createIODelegate(const std::string& format);

private:
  Session(const Session&); // Not implemented.
  void operator = (const Session&); // Not implemented.
};

// ++ 3 ++
template<typename T>
T* EntityHandle::object() const
{
  // Never return the pointer if the other data is invalid:
  if (
    !this->m_session ||
    !this->m_object ||
    this->m_modelNumber < 0 ||
    this->m_modelNumber > static_cast<int>(this->m_session->numberOfModels()))
    return NULL;

  return dynamic_cast<T*>(this->m_object.GetPointer());
}
// -- 3 --

// ++ 4 ++
template<typename T>
void EntityHandle::appendChildrenTo(T& container, int depth) const
{
  if (!this->m_session)
    return;

  vtkMultiBlockDataSet* data = this->object<vtkMultiBlockDataSet>();
  // For now, leaf-datasets have no children.
  // In the future, this may change to include model-faces/edges/segments/...
  if (!data)
    return;

  int nb = data->GetNumberOfBlocks();
  for (int i = 0; i < nb; ++i)
    {
    vtkDataObject* childData = data->GetBlock(i);
    if (!childData)
      continue;

    EntityHandle child(this->m_modelNumber, childData, data, i, this->m_session);
    container.insert(container.end(), child);

    vtkMultiBlockDataSet* mbds = vtkMultiBlockDataSet::SafeDownCast(childData);
    if (mbds && depth != 0)
      {
      child.appendChildrenTo(container, depth < 0 ? depth : depth - 1);
      }
    }
}
// -- 4 --


    } // namespace exodus
  } // namespace bridge
} // namespace smtk


#endif // __smtk_session_exodus_Session_h
