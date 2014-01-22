#include "smtk/cgm/Bridge.h"
#include "smtk/cgm/TDUUID.h"

#include "smtk/util/UUID.h"

#include "smtk/model/Cursor.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Storage.h"

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
bool Bridge::addCGMEntityToStorage(
  const smtk::util::UUID& uid, smtk::model::StoragePtr storage)
{
  ToolDataUser* tdu = TDUUID::findEntityById(uid);
  RefEntity* ent = dynamic_cast<RefEntity*>(tdu);
  if (ent)
    {
    DagType dagType = ent->dag_type();
    if (dagType == DagType::body_type())            Bridge::AddBodyToStorage(uid, dynamic_cast<Body*>(ent), storage);
    else if (dagType == DagType::co_volume_type())  Bridge::AddVolumeUseToStorage(uid, dynamic_cast<CoVolume*>(ent), storage);
    else if (dagType == DagType::co_face_type())    Bridge::AddFaceUseToStorage(uid, dynamic_cast<CoFace*>(ent), storage);
    else if (dagType == DagType::co_edge_type())    Bridge::AddEdgeUseToStorage(uid, dynamic_cast<CoEdge*>(ent), storage);
    else if (dagType == DagType::co_vertex_type())  Bridge::AddVertexUseToStorage(uid, dynamic_cast<CoVertex*>(ent), storage);
    else if (dagType == DagType::shell_type())      Bridge::AddShellToStorage(uid, dynamic_cast<Shell*>(ent), storage);
    else if (dagType == DagType::loop_type())       Bridge::AddLoopToStorage(uid, dynamic_cast<Loop*>(ent), storage);
    else if (dagType == DagType::chain_type())      Bridge::AddChainToStorage(uid, dynamic_cast<Chain*>(ent), storage);
    else if (dagType == DagType::ref_volume_type()) Bridge::AddVolumeToStorage(uid, dynamic_cast<RefVolume*>(ent), storage);
    else if (dagType == DagType::ref_face_type())   Bridge::AddFaceToStorage(uid, dynamic_cast<RefFace*>(ent), storage);
    else if (dagType == DagType::ref_edge_type())   Bridge::AddEdgeToStorage(uid, dynamic_cast<RefEdge*>(ent), storage);
    else if (dagType == DagType::ref_vertex_type()) Bridge::AddVertexToStorage(uid, dynamic_cast<RefVertex*>(ent), storage);
    else
      { // Might be a RefGroup
      RefGroup* grp = dynamic_cast<RefGroup*>(ent);
      if (grp)
        {
        Bridge::AddGroupToStorage(uid, grp, storage);
        }
      else
        {
        std::cerr << "Invalid DagType for entity " << ent->entity_name().c_str() << "\n";
        return false;
        }
      }
    return true;
    }
  return false;
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

void Bridge::AddBodyToStorage(const smtk::util::UUID& uid, Body* body, smtk::model::StoragePtr storage, bool addRels)
{
  if (body)
    {
    storage->insertModel(uid, 3, 3, body->entity_name().c_str());
    if (addRels)
      {
      // Add free cells, submodels, and groups
      }
    }
}

void Bridge::AddVolumeUseToStorage(const smtk::util::UUID& uid, CoVolume* coVolume, smtk::model::StoragePtr storage, bool addRels)
{
  if (coVolume)
    {
    (void)storage;
    (void)uid;
    if (addRels)
      {
      // Add coVolume relations and arrangements
      }
    }
}

void Bridge::AddFaceUseToStorage(const smtk::util::UUID& uid, CoFace* coFace, smtk::model::StoragePtr storage, bool addRels)
{
  if (coFace)
    {
    (void)storage;
    (void)uid;
    if (addRels)
      {
      // Add coFace relations and arrangements
      }
    }
}

void Bridge::AddEdgeUseToStorage(const smtk::util::UUID& uid, CoEdge* coEdge, smtk::model::StoragePtr storage, bool addRels)
{
  if (coEdge)
    {
    (void)storage;
    (void)uid;
    if (addRels)
      {
      // Add coEdge relations and arrangements
      }
    }
}

void Bridge::AddVertexUseToStorage(const smtk::util::UUID& uid, CoVertex* coVertex, smtk::model::StoragePtr storage, bool addRels)
{
  if (coVertex)
    {
    (void)storage;
    (void)uid;
    if (addRels)
      {
      // Add coVertex relations and arrangements
      }
    }
}

void Bridge::AddShellToStorage(const smtk::util::UUID& uid, Shell* shell, smtk::model::StoragePtr storage, bool addRels)
{
  if (shell)
    {
    (void)storage;
    (void)uid;
    if (addRels)
      {
      // Add shell relations and arrangements
      }
    }
}

void Bridge::AddLoopToStorage(const smtk::util::UUID& uid, Loop* loop, smtk::model::StoragePtr storage, bool addRels)
{
  if (loop)
    {
    (void)storage;
    (void)uid;
    if (addRels)
      {
      // Add loop relations and arrangements
      }
    }
}

void Bridge::AddChainToStorage(const smtk::util::UUID& uid, Chain* chain, smtk::model::StoragePtr storage, bool addRels)
{
  if (chain)
    {
    (void)storage;
    (void)uid;
    if (addRels)
      {
      // Add chain relations and arrangements
      }
    }
}

void Bridge::AddVolumeToStorage(const smtk::util::UUID& uid, RefVolume* refVol, smtk::model::StoragePtr storage, bool addRels)
{
  if (refVol)
    {
    (void)storage;
    (void)uid;
    if (addRels)
      {
      // Add refVol relations and arrangements
      }
    }
}

void Bridge::AddFaceToStorage(const smtk::util::UUID& uid, RefFace* refFace, smtk::model::StoragePtr storage, bool addRels)
{
  if (refFace)
    {
    (void)storage;
    (void)uid;
    if (addRels)
      {
      // Add refFace relations and arrangements
      }
    }
}

void Bridge::AddEdgeToStorage(const smtk::util::UUID& uid, RefEdge* refEdge, smtk::model::StoragePtr storage, bool addRels)
{
  if (refEdge)
    {
    (void)storage;
    (void)uid;
    if (addRels)
      {
      // Add refEdge relations and arrangements
      }
    }
}

void Bridge::AddVertexToStorage(const smtk::util::UUID& uid, RefVertex* refVert, smtk::model::StoragePtr storage, bool addRels)
{
  if (refVert)
    {
    (void)storage;
    (void)uid;
    if (addRels)
      {
      // Add refVert relations and arrangements
      }
    }
}

void Bridge::AddGroupToStorage(const smtk::util::UUID& uid, RefGroup* refGroup, smtk::model::StoragePtr storage, bool addRels)
{
  if (refGroup)
    {
    (void)storage;
    (void)uid;
    if (addRels)
      {
      // Add refGroup relations and arrangements
      }
    }
}

  } // namespace cgm
} // namespace cgmsmtk
