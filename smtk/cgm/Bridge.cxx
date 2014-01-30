#include "smtk/cgm/Bridge.h"
#include "smtk/cgm/TDUUID.h"

#include "smtk/util/UUID.h"

#include "smtk/model/Cursor.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/VolumeUse.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/VertexUse.h"
#include "smtk/model/Volume.h"
#include "smtk/model/Face.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/Storage.h"
#include "smtk/model/Shell.h"
#include "smtk/model/Chain.h"
#include "smtk/model/Loop.h"

#include "RefEntity.hpp"
#include "DagType.hpp"
#include "Body.hpp"
#include "CoVolume.hpp"
#include "CoFace.hpp"
#include "CoEdge.hpp"
#include "CoVertex.hpp"
#include "Shell.hpp"
#include "Loop.hpp"
#include "Chain.hpp"
#include "RefVolume.hpp"
#include "RefFace.hpp"
#include "RefEdge.hpp"
#include "RefVertex.hpp"
#include "RefGroup.hpp"

namespace cgmsmtk {
  namespace cgm {

/**\brief Create records in \a storage that reflect the CGM \a entity.
  *
  */
smtk::model::Cursor Bridge::addCGMEntityToStorage(
  const smtk::util::UUID& uid,
  smtk::model::StoragePtr storage,
  bool addRels)
{
  ToolDataUser* tdu = TDUUID::findEntityById(uid);
  RefEntity* ent = dynamic_cast<RefEntity*>(tdu);
  if (ent)
    return Bridge::addCGMEntityToStorage(uid, ent, storage, addRels);
  /* Wishful thinking: GroupingEntity and SenseEntity do not inherit ToolDataUser
  GroupingEntity* grp = dynamic_cast<GroupingEntity*>(tdu);
  if (grp)
    return Bridge::addCGMEntityToStorage(uid, grp, storage, addRels);
  SenseEntity* sns = dynamic_cast<SenseEntity*>(tdu);
  if (sns)
    return Bridge::addCGMEntityToStorage(uid, sns, storage, addRels);
  */
  return smtk::model::Cursor();
}

/**\brief Create CGM entities for the given SMTK entities.
  *
  * This is not a complete implementation.
  */
bool Bridge::addStorageEntityToCGM(const smtk::model::Cursor& ent)
{
  (void)ent;
  return true;
}

smtk::model::Cursor Bridge::addCGMEntityToStorage(
  const smtk::util::UUID& uid, RefEntity* ent, smtk::model::StoragePtr storage, bool addRels)
{
  DagType dagType = ent->dag_type();
  if (dagType.is_valid())
    {
    switch (dagType.functional_type())
      {
    case DagType::BasicTopologyEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return Bridge::addVolumeToStorage(uid, dynamic_cast<RefVolume*>(ent), storage, addRels);
      case 2: return Bridge::addFaceToStorage(uid, dynamic_cast<RefFace*>(ent), storage, addRels);
      case 1: return Bridge::addEdgeToStorage(uid, dynamic_cast<RefEdge*>(ent), storage, addRels);
      case 0: return Bridge::addVertexToStorage(uid, dynamic_cast<RefVertex*>(ent), storage, addRels);
        }
      break;
    case DagType::SenseEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return Bridge::addVolumeUseToStorage(uid, dynamic_cast<CoVolume*>(ent), storage, addRels);
      case 2: return Bridge::addFaceUseToStorage(uid, dynamic_cast<CoFace*>(ent), storage, addRels);
      case 1: return Bridge::addEdgeUseToStorage(uid, dynamic_cast<CoEdge*>(ent), storage, addRels);
      case 0: return Bridge::addVertexUseToStorage(uid, dynamic_cast<CoVertex*>(ent), storage, addRels);
        }
      break;
    case DagType::GroupingEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return Bridge::addBodyToStorage(uid, dynamic_cast<Body*>(ent), storage, addRels);
      case 2: return Bridge::addShellToStorage(uid, dynamic_cast<Shell*>(ent), storage, addRels);
      case 1: return Bridge::addLoopToStorage(uid, dynamic_cast<Loop*>(ent), storage, addRels);
      case 0: return Bridge::addChainToStorage(uid, dynamic_cast<Chain*>(ent), storage, addRels);
        }
      break;
      }
    }
  // Might be a RefGroup
  RefGroup* grp = dynamic_cast<RefGroup*>(ent);
  if (grp) return Bridge::addGroupToStorage(uid, grp, storage, addRels);

  // Nothing we know about
  std::cerr
    << "Invalid DagType"
    << "(" << dagType.dimension() << "," << dagType.functional_type() << ")"
    << " for entity \"" << ent->entity_name().c_str() << "\" " << ent << "\n";
  return smtk::model::Cursor();
}

smtk::model::Cursor Bridge::addCGMEntityToStorage(
  const smtk::util::UUID& uid, GroupingEntity* grp, smtk::model::StoragePtr storage, bool addRels)
{
  DagType dagType = grp->dag_type();
  if (dagType.is_valid())
    {
    switch (dagType.functional_type())
      {
    case DagType::BasicTopologyEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return Bridge::addVolumeToStorage(uid, dynamic_cast<RefVolume*>(grp), storage, addRels);
      case 2: return Bridge::addFaceToStorage(uid, dynamic_cast<RefFace*>(grp), storage, addRels);
      case 1: return Bridge::addEdgeToStorage(uid, dynamic_cast<RefEdge*>(grp), storage, addRels);
      case 0: return Bridge::addVertexToStorage(uid, dynamic_cast<RefVertex*>(grp), storage, addRels);
        }
      break;
    case DagType::SenseEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return Bridge::addVolumeUseToStorage(uid, dynamic_cast<CoVolume*>(grp), storage, addRels);
      case 2: return Bridge::addFaceUseToStorage(uid, dynamic_cast<CoFace*>(grp), storage, addRels);
      case 1: return Bridge::addEdgeUseToStorage(uid, dynamic_cast<CoEdge*>(grp), storage, addRels);
      case 0: return Bridge::addVertexUseToStorage(uid, dynamic_cast<CoVertex*>(grp), storage, addRels);
        }
      break;
    case DagType::GroupingEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return Bridge::addBodyToStorage(uid, dynamic_cast<Body*>(grp), storage, addRels);
      case 2: return Bridge::addShellToStorage(uid, dynamic_cast<Shell*>(grp), storage, addRels);
      case 1: return Bridge::addLoopToStorage(uid, dynamic_cast<Loop*>(grp), storage, addRels);
      case 0: return Bridge::addChainToStorage(uid, dynamic_cast<Chain*>(grp), storage, addRels);
        }
      break;
      }
    }
  // Might be a RefGroup
  RefGroup* refGroup = dynamic_cast<RefGroup*>(grp);
  if (refGroup) return Bridge::addGroupToStorage(uid, refGroup, storage, addRels);

  // Nothing we know about
  std::cerr
    << "Invalid DagType"
    << "(" << dagType.dimension() << "," << dagType.functional_type() << ")"
    << " for entity " << grp << "\n";
  return smtk::model::Cursor();
}

smtk::model::Cursor Bridge::addCGMEntityToStorage(
  const smtk::util::UUID& uid, SenseEntity* sns, smtk::model::StoragePtr storage, bool addRels)
{
  DagType dagType = sns->dag_type();
  if (dagType.is_valid())
    {
    switch (dagType.functional_type())
      {
    case DagType::BasicTopologyEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return Bridge::addVolumeToStorage(uid, dynamic_cast<RefVolume*>(sns), storage, addRels);
      case 2: return Bridge::addFaceToStorage(uid, dynamic_cast<RefFace*>(sns), storage, addRels);
      case 1: return Bridge::addEdgeToStorage(uid, dynamic_cast<RefEdge*>(sns), storage, addRels);
      case 0: return Bridge::addVertexToStorage(uid, dynamic_cast<RefVertex*>(sns), storage, addRels);
        }
      break;
    case DagType::SenseEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return Bridge::addVolumeUseToStorage(uid, dynamic_cast<CoVolume*>(sns), storage, addRels);
      case 2: return Bridge::addFaceUseToStorage(uid, dynamic_cast<CoFace*>(sns), storage, addRels);
      case 1: return Bridge::addEdgeUseToStorage(uid, dynamic_cast<CoEdge*>(sns), storage, addRels);
      case 0: return Bridge::addVertexUseToStorage(uid, dynamic_cast<CoVertex*>(sns), storage, addRels);
        }
      break;
    case DagType::GroupingEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return Bridge::addBodyToStorage(uid, dynamic_cast<Body*>(sns), storage, addRels);
      case 2: return Bridge::addShellToStorage(uid, dynamic_cast<Shell*>(sns), storage, addRels);
      case 1: return Bridge::addLoopToStorage(uid, dynamic_cast<Loop*>(sns), storage, addRels);
      case 0: return Bridge::addChainToStorage(uid, dynamic_cast<Chain*>(sns), storage, addRels);
        }
      break;
      }
    }
  // Might be a RefGroup
  RefGroup* grp = dynamic_cast<RefGroup*>(sns);
  if (grp) return Bridge::addGroupToStorage(uid, grp, storage, addRels);

  // Nothing we know about
  std::cerr
    << "Invalid DagType"
    << "(" << dagType.dimension() << "," << dagType.functional_type() << ")"
    << " for entity " << sns << "\n";
  return smtk::model::Cursor();
}

/// Given a CGM \a body tagged with \a uid, create a record in \a storage for it.
smtk::model::ModelEntity Bridge::addBodyToStorage(
  const smtk::util::UUID& uid,
  Body* body,
  smtk::model::StoragePtr storage,
  bool addRels)
{
  if (body)
    {
    storage->insertModel(uid, 3, 3, body->entity_name().c_str());
    if (addRels)
      {
      // Add free cells, submodels, and groups.
      // Since CGM does not allow submodels, there's nothing to do for that.
      // However, we can see what groups *contain* this body.
      DLIList<RefEntity*> parents;
      body->get_parent_ref_entities(parents);
      std::cout << "Body has " << parents.size() << " parents.\n";
      }
    return smtk::model::ModelEntity(storage, uid);
    }
  return smtk::model::ModelEntity();
}

/// Given a CGM \a coVolume tagged with \a uid, create a record in \a storage for it.
smtk::model::VolumeUse Bridge::addVolumeUseToStorage(
  const smtk::util::UUID& uid,
  CoVolume* coVolume,
  smtk::model::StoragePtr storage,
  bool addRels)
{
  if (coVolume)
    {
    if (addRels)
      {
      // Add coVolume relations and arrangements
      }
    return smtk::model::VolumeUse(storage, uid);
    }
  return smtk::model::VolumeUse();
}

/// Given a CGM \a coFace tagged with \a uid, create a record in \a storage for it.
smtk::model::FaceUse Bridge::addFaceUseToStorage(
  const smtk::util::UUID& uid,
  CoFace* coFace,
  smtk::model::StoragePtr storage,
  bool addRels)
{
  if (coFace)
    {
    if (addRels)
      {
      // Add coFace relations and arrangements
      }
    return smtk::model::FaceUse(storage, uid);
    }
  return smtk::model::FaceUse();
}

/// Given a CGM \a coEdge tagged with \a uid, create a record in \a storage for it.
smtk::model::EdgeUse Bridge::addEdgeUseToStorage(
  const smtk::util::UUID& uid,
  CoEdge* coEdge,
  smtk::model::StoragePtr storage,
  bool addRels)
{
  if (coEdge)
    {
    if (addRels)
      {
      // Add coEdge relations and arrangements
      }
    return smtk::model::EdgeUse(storage, uid);
    }
  return smtk::model::EdgeUse();
}

/// Given a CGM \a coVertex tagged with \a uid, create a record in \a storage for it.
smtk::model::VertexUse Bridge::addVertexUseToStorage(
  const smtk::util::UUID& uid,
  CoVertex* coVertex,
  smtk::model::StoragePtr storage,
  bool addRels)
{
  if (coVertex)
    {
    if (addRels)
      {
      // Add coVertex relations and arrangements
      }
    return smtk::model::VertexUse(storage, uid);
    }
  return smtk::model::VertexUse();
}

/// Given a CGM \a shell tagged with \a uid, create a record in \a storage for it.
smtk::model::Shell Bridge::addShellToStorage(
  const smtk::util::UUID& uid,
  Shell* shell,
  smtk::model::StoragePtr storage,
  bool addRels)
{
  if (shell)
    {
    if (addRels)
      {
      // Add shell relations and arrangements
      }
    return smtk::model::Shell(storage, uid);
    }
  return smtk::model::Shell();
}

/// Given a CGM \a loop tagged with \a uid, create a record in \a storage for it.
smtk::model::Loop Bridge::addLoopToStorage(
  const smtk::util::UUID& uid,
  Loop* loop,
  smtk::model::StoragePtr storage,
  bool addRels)
{
  if (loop)
    {
    if (addRels)
      {
      // Add loop relations and arrangements
      }
    return smtk::model::Loop(storage, uid);
    }
  return smtk::model::Loop();
}

/// Given a CGM \a chain tagged with \a uid, create a record in \a storage for it.
smtk::model::Chain Bridge::addChainToStorage(
  const smtk::util::UUID& uid,
  Chain* chain,
  smtk::model::StoragePtr storage,
  bool addRels)
{
  if (chain)
    {
    if (addRels)
      {
      // Add chain relations and arrangements
      }
    return smtk::model::Chain(storage, uid);
    }
  return smtk::model::Chain();
}

/// Given a CGM \a refVolume tagged with \a uid, create a record in \a storage for it.
smtk::model::Volume Bridge::addVolumeToStorage(
  const smtk::util::UUID& uid,
  RefVolume* refVolume,
  smtk::model::StoragePtr storage,
  bool addRels)
{
  if (refVolume)
    {
    if (addRels)
      {
      // Add refVolume relations and arrangements
      }
    return smtk::model::Volume(storage, uid);
    }
  return smtk::model::Volume();
}

/// Given a CGM \a refFace tagged with \a uid, create a record in \a storage for it.
smtk::model::Face Bridge::addFaceToStorage(
  const smtk::util::UUID& uid,
  RefFace* refFace,
  smtk::model::StoragePtr storage,
  bool addRels)
{
  if (refFace)
    {
    if (addRels)
      {
      // Add refFace relations and arrangements
      }
    return smtk::model::Face(storage, uid);
    }
  return smtk::model::Face();
}

/// Given a CGM \a refEdge tagged with \a uid, create a record in \a storage for it.
smtk::model::Edge Bridge::addEdgeToStorage(
  const smtk::util::UUID& uid,
  RefEdge* refEdge,
  smtk::model::StoragePtr storage,
  bool addRels)
{
  if (refEdge)
    {
    if (addRels)
      {
      // Add refEdge relations and arrangements
      }
    return smtk::model::Edge(storage, uid);
    }
  return smtk::model::Edge(storage, uid);
}

/// Given a CGM \a refVertex tagged with \a uid, create a record in \a storage for it.
smtk::model::Vertex Bridge::addVertexToStorage(
  const smtk::util::UUID& uid,
  RefVertex* refVertex,
  smtk::model::StoragePtr storage,
  bool addRels)
{
  if (refVertex)
    {
    if (addRels)
      {
      // Add refVertex relations and arrangements
      }
    return smtk::model::Vertex(storage, uid);
    }
  return smtk::model::Vertex();
}

/// Given a CGM \a refGroup tagged with \a uid, create a record in \a storage for it.
smtk::model::GroupEntity Bridge::addGroupToStorage(
  const smtk::util::UUID& uid,
  RefGroup* refGroup,
  smtk::model::StoragePtr storage,
  bool addRels)
{
  if (refGroup)
    {
    (void)storage;
    (void)uid;
    if (addRels)
      {
      // Add refGroup relations and arrangements
      }
    return smtk::model::GroupEntity(storage, uid);
    }
  return smtk::model::GroupEntity();
}

  } // namespace cgm
} // namespace cgmsmtk
