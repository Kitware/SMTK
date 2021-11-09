//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_vtk_Session_h
#define smtk_session_vtk_Session_h

#include "smtk/session/vtk/Exports.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Session.h"

#include "smtk/common/UUID.h"

#include "vtkDataObjectTreeIterator.h"
#include "vtkInformation.h"
#include "vtkInformationObjectBaseVectorKey.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

#include <map>
#include <vector>

class vtkInformationDoubleKey;
class vtkInformationIntegerKey;
class vtkInformationStringKey;

namespace smtk
{
namespace session
{
namespace vtk
{

class Session;
typedef smtk::shared_ptr<Session> SessionPtr;

// ++ 2 ++
/// The types of entities in a VTK "model"
enum EntityType
{
  EXO_MODEL = 0x01,    //!< An entire VTK dataset (a file).
  EXO_BLOCK = 0x02,    //!< VTK BLOCKs are groups of cells inside a MODEL.
  EXO_SIDE_SET = 0x03, //!< VTK SIDE_SETs are groups of cell boundaries in a MODEL.
  EXO_NODE_SET = 0x04, //!< VTK NODE_SETs are groups of points in a MODEL.

  EXO_BLOCKS = 0x05,    //!< A group of VTK BLOCKs.
  EXO_SIDE_SETS = 0x06, //!< A group of VTK SIDE_SETs.
  EXO_NODE_SETS = 0x07, //!< A group of VTK NODE_SETs.

  EXO_LABEL_MAP = 0x08, //!< A dataset with a label-map array
  EXO_LABEL = 0x09,     //!< A dataset representing one label in a label-map array

  EXO_INVALID = 0xff //!< The handle is invalid
};

SMTKVTKSESSION_EXPORT std::string EntityTypeNameString(EntityType etype);

/// A "handle" for a VTK entity (file, block, side set, or node set)
struct SMTKVTKSESSION_EXPORT EntityHandle
{
  int m_modelNumber{
    -1
  }; //!< An offset in the vector of models (m_models) owned by the session, whose model owns m_object.
  vtkSmartPointer<vtkDataObject> m_object; //!< The dataset being presented as this entity.
  SessionPtr m_session;                    //!< The session owning this entity.

  EntityHandle();
  EntityHandle(int emod, vtkDataObject* obj, SessionPtr sess);
  EntityHandle(
    int emod,
    vtkDataObject* obj,
    vtkDataObject* parent,
    int idxInParent,
    SessionPtr sess);

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

  bool operator==(const EntityHandle& other) const
  {
    return this->m_session == other.m_session && this->m_object == other.m_object &&
      this->m_modelNumber == other.m_modelNumber;
  }
  bool operator!=(const EntityHandle& other) const
  {
    return this->m_session != other.m_session || this->m_object != other.m_object ||
      this->m_modelNumber != other.m_modelNumber;
  }
};
// -- 2 --

typedef std::vector<EntityHandle> EntityHandleArray;

// ++ 1 ++
/**\brief Implement a session from VTK mesh files to SMTK.
  *
  * This session uses the VTK VTK reader to obtain
  * information from VTK files, with each element block,
  * side set, and node set represented as a vtkUnstructuredGrid.
  */
class SMTKVTKSESSION_EXPORT Session : public smtk::model::Session
{
public:
  typedef std::vector<vtkSmartPointer<vtkMultiBlockDataSet>> ModelVector_t;
  typedef std::map<smtk::model::EntityRef, EntityHandle> ReverseIdMap_t;
  typedef std::pair<vtkDataObject*, int> ParentAndIndex_t;
  typedef std::map<vtkDataObject*, ParentAndIndex_t> ChildParentMap_t;

  smtkTypeMacro(Session);
  smtkSuperclassMacro(smtk::model::Session);
  smtkSharedFromThisMacro(smtk::model::Session);
  smtkCreateMacro(smtk::model::Session);
  typedef smtk::model::SessionInfoBits SessionInfoBits;
  ~Session() override;
  SessionInfoBits allSupportedInformation() const override
  {
    return smtk::model::SESSION_EVERYTHING;
  }

  Session(const Session&) = delete;
  Session& operator=(const Session&) = delete;

  EntityHandle toEntity(const smtk::model::EntityRef& eid);
  smtk::model::EntityRef toEntityRef(const EntityHandle& ent);

  // VTK keys used to mark blocks.
  static vtkInformationIntegerKey* SMTK_DIMENSION();
  static vtkInformationIntegerKey* SMTK_VISIBILITY();
  static vtkInformationIntegerKey* SMTK_GROUP_TYPE();
  static vtkInformationIntegerKey* SMTK_PEDIGREE();
  static vtkInformationIntegerKey* SMTK_OUTER_LABEL();
  static vtkInformationObjectBaseVectorKey* SMTK_CHILDREN();
  static vtkInformationDoubleKey* SMTK_LABEL_VALUE();

  smtk::model::Model addModel(
    vtkSmartPointer<vtkMultiBlockDataSet>& model,
    SessionInfoBits requestedInfo = smtk::model::SESSION_EVERYTHING);

  std::string defaultFileExtension(const smtk::model::Model& model) const override;

  bool ensureChildParentMapEntry(vtkDataObject* child, vtkDataObject* parent, int idxInParent);

  ReverseIdMap_t& reverseIdMap() { return m_revIdMap; }

  size_t numberOfModels() const;

  /// Get/set whether adding cells to models during transcription will check for pre-existing
  /// entries to avoid adding duplicates. This defaults to true, but some operations such as
  /// imports may wish to diable for efficiency (if adding many cells to a model that cannot
  /// already be present on the model).
  static bool transcriptionChecksEnabled() { return s_transcriptionChecks; }
  static void setEnableTranscriptionChecks(bool doCheck) { s_transcriptionChecks = doCheck; }

protected:
  friend struct EntityHandle;

  Session();

  SessionInfoBits transcribeInternal(
    const smtk::model::EntityRef& entity,
    SessionInfoBits requestedInfo,
    int depth = -1) override;

  ModelVector_t m_models;
  ReverseIdMap_t m_revIdMap;
  ChildParentMap_t
    m_cpMap; // vtkMultiBlockDataSet doesn't provide a fast way to obtain parent of leaf datasets.
  // std::map<EntityHandle,smtk::model::EntityRef> m_fwdIdMap; // not needed; store UUID in vtkInformation.
  // -- 1 --
  static bool s_transcriptionChecks;

  vtkDataObject* modelOfHandle(const EntityHandle& h) const;
  vtkDataObject* parent(vtkDataObject* obj) const;
  int parentIndex(vtkDataObject* obj) const;

  smtk::common::UUID uuidOfHandleObject(vtkDataObject* obj) const;

  template<typename T>
  T* modelOfHandleAs(const EntityHandle& h) const
  {
    return T::SafeDownCast(this->modelOfHandle(h));
  }

  template<typename T>
  T* parentAs(vtkDataObject* obj) const
  {
    return T::SafeDownCast(this->parent(obj));
  }

  smtk::model::SessionIOPtr createIODelegate(const std::string& format) override;
};

// ++ 3 ++
template<typename T>
T* EntityHandle::object() const
{
  // Never return the pointer if the other data is invalid:
  if (
    !this->m_session || !this->m_object || this->m_modelNumber < 0 ||
    this->m_modelNumber > static_cast<int>(this->m_session->numberOfModels()))
  {
    return nullptr;
  }

  return dynamic_cast<T*>(this->m_object.GetPointer());
}
// -- 3 --

// ++ 4 ++
template<typename T>
void EntityHandle::appendChildrenTo(T& container, int depth) const
{
  if (!this->m_session)
    return;

  vtkDataObject* obj = this->object<vtkDataObject>();
  if (!obj)
    return;

  vtkInformation* info = obj->GetInformation();
  int objkids = Session::SMTK_CHILDREN()->Length(info);
  vtkMultiBlockDataSet* data = vtkMultiBlockDataSet::SafeDownCast(obj);
  if (!data && objkids < 1)
    return;

  if (data)
  {
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
  if (objkids > 0)
  {
    for (int i = 0; i < objkids; ++i)
    {
      vtkDataObject* childData =
        vtkDataObject::SafeDownCast(Session::SMTK_CHILDREN()->Get(info, i));
      if (!childData)
        continue;

      EntityHandle child(this->m_modelNumber, childData, obj, i, this->m_session);
      container.insert(container.end(), child);

      // Recurse here to see if children have children...
      if (depth != 0 && Session::SMTK_CHILDREN()->Length(childData->GetInformation()) > 0)
      {
        child.appendChildrenTo(container, depth < 0 ? depth : depth - 1);
      }
    }
  }
}
// -- 4 --

} // namespace vtk
} // namespace session
} // namespace smtk

#endif // smtk_session_vtk_Session_h
