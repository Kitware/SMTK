//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/session/vtk/Session.h"

#include "smtk/common/UUIDGenerator.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Tessellation.h"

#include "smtk/extension/vtk/source/vtkResourceMultiBlockSource.h"

#include "vtkCellArray.h"
#include "vtkGeometryFilter.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationIntegerVectorKey.h"
#include "vtkInformationObjectBaseVectorKey.h"
#include "vtkInformationStringKey.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkUnsignedIntArray.h"

using namespace smtk::model;
using namespace smtk::common;

namespace smtk
{
namespace session
{
namespace vtk
{

vtkInformationKeyMacro(Session, SMTK_DIMENSION, Integer);
vtkInformationKeyMacro(Session, SMTK_VISIBILITY, Integer);
vtkInformationKeyMacro(Session, SMTK_GROUP_TYPE, Integer);
vtkInformationKeyMacro(Session, SMTK_PEDIGREE, Integer);
vtkInformationKeyMacro(Session, SMTK_OUTER_LABEL, Integer);
vtkInformationKeyMacro(Session, SMTK_CHILDREN, ObjectBaseVector);
vtkInformationKeyMacro(Session, SMTK_LABEL_VALUE, Double);

enum smtkCellTessRole
{
  SMTK_ROLE_VERTS,
  SMTK_ROLE_LINES,
  SMTK_ROLE_POLYS
};

/// Return a string representing the type of object
std::string EntityTypeNameString(EntityType etype)
{
  switch (etype)
  {
    case EXO_MODEL:
      return "model";

    case EXO_BLOCK:
      return "element block";
    case EXO_SIDE_SET:
      return "side set";
    case EXO_NODE_SET:
      return "node set";

    case EXO_BLOCKS:
      return "element blocks";
    case EXO_SIDE_SETS:
      return "side sets";
    case EXO_NODE_SETS:
      return "node sets";

    case EXO_LABEL_MAP:
      return "label map";
    case EXO_LABEL:
      return "label";

    default:
      break;
  }
  return "invalid";
}

/// Construct an invalid handle.
EntityHandle::EntityHandle()
  : m_modelNumber(-1)
  , m_object(nullptr)
  , m_session(nullptr)
{
}

/// Construct a possibly-valid handle (of a top-level model).
EntityHandle::EntityHandle(int emod, vtkDataObject* obj, SessionPtr sess)
  : m_modelNumber(emod)
  , m_object(obj)
  , m_session(sess)
{
}

/// Construct a possibly-valid handle (of a non-top-level entity).
EntityHandle::EntityHandle(
  int emod, vtkDataObject* obj, vtkDataObject* parent, int idxInParent, SessionPtr sess)
  : m_modelNumber(emod)
  , m_object(obj)
  , m_session(sess)
{
  if (sess && obj && parent && idxInParent >= 0)
  {
    sess->ensureChildParentMapEntry(obj, parent, idxInParent);
  }
}

/// Returns true when the object is owned by a session and has a non-NULL pointer.
bool EntityHandle::isValid() const
{
  return this->m_session && this->m_object && this->m_modelNumber >= 0 &&
    this->m_modelNumber < static_cast<int>(this->m_session->numberOfModels());
}

/// Return the type of object this handle represents (or EXO_INVALID).
EntityType EntityHandle::entityType() const
{
  vtkDataObject* obj = this->object<vtkDataObject>();
  if (!obj)
    return EXO_INVALID;

  int etype = obj->GetInformation()->Get(Session::SMTK_GROUP_TYPE());
  return etype > 0 ? static_cast<EntityType>(etype) : EXO_INVALID;
}

/// Return the name assigned to this object.
/// Note that this is *not* the same as the block name that VTK uses!
std::string EntityHandle::name() const
{
  vtkDataObject* obj = this->object<vtkDataObject>();
  if (!obj)
    return std::string();

  const char* val = obj->GetInformation()->Get(vtkCompositeDataSet::NAME());
  return val ? std::string(val) : std::string();
}

/// Return the pedigree ID assigned to this object.
/// For Exodus files, this is the block or set ID. For SLAC files, it is the block index.
int EntityHandle::pedigree() const
{
  vtkDataObject* obj = this->object<vtkDataObject>();
  if (!obj)
    return -1;

  return obj->GetInformation()->Get(Session::SMTK_PEDIGREE());
}

/// Return the default visibility assigned to this object.
bool EntityHandle::visible() const
{
  vtkDataObject* obj = this->object<vtkDataObject>();
  if (!obj)
    return true; // Visible by default

  int eprop = obj->GetInformation()->Get(Session::SMTK_VISIBILITY());
  // When eprop is 0, the property was not present (or was set to 0).
  // In that case, assume the object is visible.
  // If eprop is set, it should be either -1 (invisible) or +1 (visible):
  return eprop == 0 ? true : (eprop >= 0);
}

/// Given a handle, return its parent if it has one.
EntityHandle EntityHandle::parent() const
{
  EntityType etype = this->entityType();
  // Top-level and invalid handles have an invalid parent.
  if (etype == EXO_MODEL || etype == EXO_INVALID)
    return EntityHandle();

  return EntityHandle(
    this->m_modelNumber, this->m_session->parent(this->m_object), this->m_session);
}

Session::Session() = default;

Session::~Session() = default;

// ++ 3 ++
/// Turn any valid entityref into an entity handle.
EntityHandle Session::toEntity(const smtk::model::EntityRef& eid)
{
  ReverseIdMap_t::const_iterator it = this->m_revIdMap.find(eid);
  if (it == this->m_revIdMap.end())
    return EntityHandle();
  return it->second;
}
// -- 3 --

// ++ 4 ++
smtk::model::EntityRef Session::toEntityRef(const EntityHandle& ent)
{
  vtkDataObject* entData = ent.object<vtkDataObject>();
  if (!entData)
    return EntityRef(); // an invalid entityref

  smtk::common::UUID uid = Session::uuidOfHandleObject(entData);
  return EntityRef(this->resource(), uid);
}
// -- 4 --

// ++ 6 ++
/// Add the dataset and its blocks to the session.
smtk::model::Model Session::addModel(
  vtkSmartPointer<vtkMultiBlockDataSet>& model, SessionInfoBits requestedInfo)
{
  EntityHandle handle(
    static_cast<int>(this->m_models.size()), model.GetPointer(), shared_from_this());
  this->m_models.push_back(model);
  smtk::model::Model result = this->toEntityRef(handle);
  this->m_revIdMap[result] = handle;
  this->transcribe(result, requestedInfo, false);
  result.setSession(smtk::model::SessionRef(this->resource(), this->sessionId()));
  return result;
}
// -- 6 --

std::string Session::defaultFileExtension(const smtk::model::Model& model) const
{
  std::string result = ".simple";
  if (model.hasStringProperty("type"))
  {
    const StringList& tval(model.stringProperty("type"));
    if (!tval.empty())
    {
      result = tval[0];
      if (result == "slac")
        return ".ncdf";
      else if (result == "exodus")
        return ".exo";
    }
  }
  return result;
}

void AddCellToParent(smtk::model::EntityRef& mutableEntityRef, EntityHandle& handle, Session* sess)
{
  if (mutableEntityRef.isCellEntity())
  {
    EntityHandle pp = handle;
    while (pp.entityType() != EXO_INVALID)
    {
      if (pp.parent().entityType() != EXO_INVALID)
      {
        pp = pp.parent();
      }
      else
      {
        break;
      }
    }
    if (pp != handle)
    {
      sess->toEntityRef(pp).as<smtk::model::Model>().addCell(mutableEntityRef, false);
    }
  }
}

// ++ 7 ++
SessionInfoBits Session::transcribeInternal(
  const smtk::model::EntityRef& entity, SessionInfoBits requestedInfo, int depth)
{
  SessionInfoBits actual = SESSION_NOTHING;
  EntityHandle handle = this->toEntity(entity);
  if (!handle.isValid())
    return actual;

  vtkDataObject* obj = handle.object<vtkDataObject>();
  // ...
  // -- 7 --
  if (!obj)
    return actual;

  int dim = obj->GetInformation()->Get(Session::SMTK_DIMENSION());

  // Grab the parent entity early if possible...
  EntityRef parentEntityRef;
  EntityHandle parentHandle = handle.parent();
  if (parentHandle.isValid())
  {
    parentEntityRef = this->toEntityRef(parentHandle);
    if (!parentEntityRef.isValid())
    {
      // The handle is valid, so perhaps we were asked to
      // transcribe a group before its parent model?
      this->declareDanglingEntity(parentEntityRef, 0);
      this->transcribe(parentEntityRef, requestedInfo, true, depth < 0 ? depth : depth - 1);
    }
  }

  // ++ 8 ++
  smtk::model::EntityRef mutableEntityRef(entity);
  BitFlags entityDimBits;
  if (!mutableEntityRef.isValid())
  {
    // -- 8 --
    // ++ 9 ++
    switch (handle.entityType())
    {
      case EXO_MODEL:
        mutableEntityRef.resource()->insertModel(mutableEntityRef.entity(), dim, dim);
        mutableEntityRef.setIntegerProperty(SMTK_GEOM_STYLE_PROP, smtk::model::DISCRETE);
        break;
      case EXO_LABEL_MAP:
      case EXO_LABEL:
      case EXO_BLOCK:
        mutableEntityRef.resource()->addCellOfDimensionWithUUID(mutableEntityRef.entity(), dim);
        mutableEntityRef.setName(handle.name());
        AddCellToParent(mutableEntityRef, handle, this);
        break;
      // .. and other cases.
      // -- 9 --
      case EXO_SIDE_SET:
        mutableEntityRef.resource()->addCellOfDimensionWithUUID(mutableEntityRef.entity(), dim);
        mutableEntityRef.setName(handle.name());
        AddCellToParent(mutableEntityRef, handle, this);
        break;
      case EXO_NODE_SET:
        mutableEntityRef.resource()->addCellOfDimensionWithUUID(mutableEntityRef.entity(), 0);
        mutableEntityRef.setName(handle.name());
        AddCellToParent(mutableEntityRef, handle, this);
        break;

      // Groups of groups:
      case EXO_BLOCKS:
        entityDimBits = Entity::dimensionToDimensionBits(dim);
        mutableEntityRef.resource()->insertGroup(
          mutableEntityRef.entity(), GROUP_ENTITY | entityDimBits, handle.name());
        mutableEntityRef.as<Group>().setMembershipMask(VOLUME | GROUP_ENTITY);
        break;
      case EXO_SIDE_SETS:
        entityDimBits = 0;
        for (int i = 0; i <= dim; ++i)
          entityDimBits |= Entity::dimensionToDimensionBits(i);
        mutableEntityRef.resource()->insertGroup(
          mutableEntityRef.entity(), GROUP_ENTITY | entityDimBits, handle.name());
        mutableEntityRef.as<Group>().setMembershipMask(CELL_ENTITY | GROUP_ENTITY | entityDimBits);
        break;
      case EXO_NODE_SETS:
        mutableEntityRef.resource()->insertGroup(
          mutableEntityRef.entity(), GROUP_ENTITY | DIMENSION_0, handle.name());
        mutableEntityRef.as<Group>().setMembershipMask(VERTEX | GROUP_ENTITY);
        break;
      // ++ 10 ++
      default:
        return actual;
        break;
    }
    actual |= smtk::model::SESSION_ENTITY_TYPE;
  }
  else
  {
    // If the entity is valid, is there any reason to refresh it?
    // Perhaps we want additional information transcribed?
    if (this->danglingEntities().find(mutableEntityRef) == this->danglingEntities().end())
    {
      return smtk::model::
        SESSION_EVERYTHING; // Not listed as dangling => everything transcribed already.
    }
  }
  // -- 10 --

  // ++ 11 ++
  if (requestedInfo & (smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS))
  {
    if (parentEntityRef.isValid())
    { // Connect this entity to its parent.
      mutableEntityRef.findOrAddRawRelation(parentEntityRef);
    }
    // Now add children.
    EntityHandleArray children =
      handle.childrenAs<EntityHandleArray>(0); // Only immediate children.
    EntityHandleArray::iterator cit;
    for (cit = children.begin(); cit != children.end(); ++cit)
    {
      EntityRef childEntityRef = this->toEntityRef(*cit);
      if (!childEntityRef.isValid())
      {
        this->m_revIdMap[childEntityRef] = *cit;
        this->declareDanglingEntity(childEntityRef, 0);
        this->transcribeInternal(childEntityRef, requestedInfo, depth < 0 ? depth : depth - 1);
      }
      if (handle.entityType() == EXO_MODEL)
      {
        if (childEntityRef.isCellEntity())
          mutableEntityRef.as<smtk::model::Model>().addCell(childEntityRef);
        else if (childEntityRef.isGroup())
          mutableEntityRef.as<smtk::model::Model>().addGroup(childEntityRef);
      }
      else
      {
        mutableEntityRef.as<smtk::model::Group>().addEntity(childEntityRef);
        if (childEntityRef.isCellEntity())
          mutableEntityRef.owningModel().addCell(childEntityRef);
      }
    }

    // Mark that we added this information to the resource:
    actual |= smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS;
  }
  // -- 11 --
  if (requestedInfo & smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS)
  {
    // FIXME: Todo.
    actual |= smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS;
  }
  if (requestedInfo & smtk::model::SESSION_PROPERTIES)
  {
    // Set properties.
    EntityType etype = handle.entityType();
    if (!handle.visible())
    {
      mutableEntityRef.setIntegerProperty("visible", 0);
    }
    switch (etype)
    {
      case EXO_BLOCK:
        mutableEntityRef.setStringProperty("_simple type", "element block");
        mutableEntityRef.setIntegerProperty("pedigree id", handle.pedigree());
        break;
      case EXO_NODE_SET:
        mutableEntityRef.setStringProperty("_simple type", "node set");
        mutableEntityRef.setIntegerProperty("pedigree id", handle.pedigree());
        break;
      case EXO_SIDE_SET:
        mutableEntityRef.setStringProperty("_simple type", "side set");
        mutableEntityRef.setIntegerProperty("pedigree id", handle.pedigree());
        break;
      case EXO_BLOCKS:
        mutableEntityRef.setStringProperty("_simple type", "element block collection");
        break;
      case EXO_NODE_SETS:
        mutableEntityRef.setStringProperty("_simple type", "node set collection");
        break;
      case EXO_SIDE_SETS:
        mutableEntityRef.setStringProperty("_simple type", "side set collection");
        break;

      case EXO_LABEL:
        mutableEntityRef.setStringProperty("_simple type", "label");
        mutableEntityRef.as<smtk::model::Group>().setMembershipMask(DIMENSION_3 | MODEL_DOMAIN);
        mutableEntityRef.setIntegerProperty("pedigree id", handle.pedigree());
        break;
      case EXO_LABEL_MAP:
        mutableEntityRef.setStringProperty("_simple type", "label map");
        mutableEntityRef.as<smtk::model::Group>().setMembershipMask(DIMENSION_3 | MODEL_DOMAIN);
        mutableEntityRef.setIntegerProperty("pedigree id", handle.pedigree());
        break;

      case EXO_MODEL:
        mutableEntityRef.setStringProperty("_simple type", "file");
        mutableEntityRef.setIntegerProperty("file order", handle.pedigree());
        break;
      default:
        break;
    }
    mutableEntityRef.setName(handle.name());

    actual |= smtk::model::SESSION_PROPERTIES;
  }

  this->declareDanglingEntity(mutableEntityRef, actual);
  return actual;
}

size_t Session::numberOfModels() const
{
  return this->m_models.size();
}

/// Return the model owning the given handle, \a h.
vtkDataObject* Session::modelOfHandle(const EntityHandle& h) const
{
  return (h.isValid() ? this->m_models[h.modelNumber()] : nullptr);
}

/// Return the parent dataset of \a obj.
vtkDataObject* Session::parent(vtkDataObject* obj) const
{
  ChildParentMap_t::const_iterator it = this->m_cpMap.find(obj);
  if (it == this->m_cpMap.end())
    return nullptr;

  return it->second.first;
}

/// Return the index of \a obj in its parent's list of children.
int Session::parentIndex(vtkDataObject* obj) const
{
  ChildParentMap_t::const_iterator it = this->m_cpMap.find(obj);
  if (it == this->m_cpMap.end())
    return -1;

  return it->second.second;
}

bool Session::ensureChildParentMapEntry(
  vtkDataObject* child, vtkDataObject* parent, int idxInParent)
{
  return this->m_cpMap
    .insert(ChildParentMap_t::value_type(child, ParentAndIndex_t(parent, idxInParent)))
    .second;
}

// ++ 13 ++
smtk::common::UUID Session::uuidOfHandleObject(vtkDataObject* obj) const
{
  smtk::common::UUID uid;
  if (!obj)
  {
    return uid;
  }

  uid = vtkResourceMultiBlockSource::GetDataObjectUUID(obj->GetInformation());
  if (!uid)
  { // We have not assigned a UUID yet. Do so now.
    uid = smtk::common::UUIDGenerator::instance().random();
    vtkResourceMultiBlockSource::SetDataObjectUUID(obj->GetInformation(), uid);
  }
  return uid;
}
// -- 13 --

/**\brief Return a delegate to export session-specific data.
  *
  * If your session needs to store additional information when being
  * serialized to JSON or some other format (only JSON is currently
  * supported), this method is called by the exporter to create an
  * object used to write this data.
  */
// ++ 12 ++
SessionIOPtr Session::createIODelegate(const std::string& format)
{
  SessionIOPtr result;
  if (format == "json")
  {
    result = this->Superclass::createIODelegate(format);
  }
  return result;
}
// -- 12 --

} // namespace vtk
} // namespace session
} // namespace smtk
