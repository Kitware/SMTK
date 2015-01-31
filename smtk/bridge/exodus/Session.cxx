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
#include "Session.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Tessellation.h"

#include "vtkCellArray.h"
#include "vtkGeometryFilter.h"
#include "vtkHyperTreeGrid.h"
#include "vtkInformation.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationIntegerVectorKey.h"
#include "vtkInformationStringKey.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkUnsignedIntArray.h"

using namespace smtk::model;
using namespace smtk::common;

namespace smtk {
  namespace bridge {
    namespace exodus {

vtkInformationKeyMacro(Session,SMTK_UUID_KEY,String);

enum smtkCellTessRole {
  SMTK_ROLE_VERTS,
  SMTK_ROLE_LINES,
  SMTK_ROLE_POLYS
};

// ++ 2 ++
Session::Session()
{
  this->initializeOperatorSystem(Session::s_operators);
}
// -- 2 --

Session::~Session()
{
}

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
  vtkDataObject* entData = this->toBlock<vtkDataObject>(ent);
  if (!entData)
    return EntityRef(); // an invalid entityref

  const char* uuidChar = entData->GetInformation()->Get(SMTK_UUID_KEY());
  smtk::common::UUID uid;
  if (!uuidChar)
    { // We have not assigned a UUID yet. Do so now.
    uid = this->m_uuidGen.random();
    entData->GetInformation()->Set(SMTK_UUID_KEY(), uid.toString().c_str());
    }
  else
    {
    uid = smtk::common::UUID(uuidChar);
    }
  return EntityRef(this->manager(), uid);
}
// -- 4 --

// ++ 5 ++
std::vector<EntityHandle> Session::childrenOf(const EntityHandle& ent)
{
  std::vector<EntityHandle> children;
  if (ent.entityType != EXO_MODEL)
    return children; // element blocks, side sets, and node sets have no children (yet).

  vtkMultiBlockDataSet* model = this->toBlock<vtkMultiBlockDataSet>(ent);
  if (!model)
    return children;

  struct {
    EntityType entityType;
    int blockId;
  } blocksByType[] = {
    {EXO_BLOCK,    0},
    {EXO_SIDE_SET, 4},
    {EXO_NODE_SET, 7}
  };
  const int numBlocksByType =
    sizeof(blocksByType) / sizeof(blocksByType[0]);
  for (int i = 0; i < numBlocksByType; ++i)
    {
    vtkMultiBlockDataSet* typeSet =
      dynamic_cast<vtkMultiBlockDataSet*>(
        model->GetBlock(blocksByType[i].blockId));
    if (!typeSet) continue;
    for (unsigned j = 0; j < typeSet->GetNumberOfBlocks(); ++j)
      children.push_back(
        EntityHandle(blocksByType[i].entityType, ent.modelNumber, j));
    }
  return children;
}
// -- 5 --

// ++ 6 ++
/// Add the dataset and its blocks to the session.
smtk::model::Model Session::addModel(
  vtkSmartPointer<vtkMultiBlockDataSet>& model)
{
  EntityHandle handle;
  handle.modelNumber = static_cast<int>(this->m_models.size());
  handle.entityType = EXO_MODEL;
  handle.entityId = -1; // unused for EXO_MODEL.
  this->m_models.push_back(model);
  smtk::model::Model result = this->toEntityRef(handle);
  this->m_revIdMap[result] = handle;
  this->transcribe(result, smtk::model::SESSION_EVERYTHING, false);
  this->manager()->setSessionForModel(shared_from_this(), result.entity());
  return result;
}
// -- 6 --

// ++ 7 ++
SessionInfoBits Session::transcribeInternal(
  const smtk::model::EntityRef& entity,
  SessionInfoBits requestedInfo)
{
  SessionInfoBits actual = SESSION_NOTHING;
  EntityHandle handle = this->toEntity(entity);
  if (!handle.isValid())
    return actual;

  vtkDataObject* obj = this->toBlock<vtkDataObject>(handle);
  // ...
// -- 7 --
  if (!obj)
    return actual;

  int dim = obj->GetInformation()->Get(vtkHyperTreeGrid::DIMENSION());

  // Grab the parent entity early if possible... we need its dimension().
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
      this->transcribe(parentEntityRef, requestedInfo, true);
      }
    dim = parentEntityRef.embeddingDimension();
    }

// ++ 8 ++
  smtk::model::EntityRef mutableEntityRef(entity);
  BitFlags entityDimBits;
  if (!mutableEntityRef.isValid())
    {
// -- 8 --
// ++ 9 ++
    switch (handle.entityType)
      {
    case EXO_MODEL:
      mutableEntityRef.manager()->insertModel(
        mutableEntityRef.entity(), dim, dim);
      break;
    case EXO_BLOCK:
      entityDimBits = Entity::dimensionToDimensionBits(dim);
      mutableEntityRef.manager()->insertGroup(
        mutableEntityRef.entity(), MODEL_DOMAIN | entityDimBits,
        this->toBlockName(handle));
      mutableEntityRef.as<Group>().setMembershipMask(VOLUME);
      break;
    // .. and other cases.
// -- 9 --
    case EXO_SIDE_SET:
      entityDimBits = 0;
      for (int i = 0; i < dim; ++i)
        entityDimBits |= Entity::dimensionToDimensionBits(i);
      mutableEntityRef.manager()->insertGroup(
        mutableEntityRef.entity(), MODEL_BOUNDARY | entityDimBits,
        this->toBlockName(handle));
      mutableEntityRef.as<Group>().setMembershipMask(CELL_ENTITY | entityDimBits);
      break;
    case EXO_NODE_SET:
      mutableEntityRef.manager()->insertGroup(
        mutableEntityRef.entity(), MODEL_BOUNDARY | DIMENSION_0,
        this->toBlockName(handle));
      mutableEntityRef.as<Group>().setMembershipMask(VERTEX);
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
    if (this->danglingEntities().find(mutableEntityRef) ==
      this->danglingEntities().end())
      return smtk::model::SESSION_EVERYTHING; // Not listed as dangling => everything transcribed already.
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
    std::vector<EntityHandle> children = this->childrenOf(handle);
    std::vector<EntityHandle>::iterator cit;
    for (cit = children.begin(); cit != children.end(); ++cit)
      {
      EntityRef childEntityRef = this->toEntityRef(*cit);
      if (!childEntityRef.isValid())
        {
        this->m_revIdMap[childEntityRef] = *cit;
        this->declareDanglingEntity(childEntityRef, 0);
        this->transcribe(childEntityRef, requestedInfo, true);
        }
      mutableEntityRef.as<smtk::model::Model>().addGroup(childEntityRef);
      }

    // Mark that we added this information to the manager:
    actual |= smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS;
    }
// -- 11 --
  if (requestedInfo & smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS)
    {
    // FIXME: Todo.
    actual |= smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS;
    }
  if (requestedInfo & smtk::model::SESSION_TESSELLATION)
    {
    if (this->addTessellation(entity, handle))
      actual |= smtk::model::SESSION_TESSELLATION;
    }
  if (requestedInfo & smtk::model::SESSION_PROPERTIES)
    {
    // Set properties.
    actual |= smtk::model::SESSION_PROPERTIES;
    }

  this->declareDanglingEntity(mutableEntityRef, actual);
  return actual;
}

/// Return the block name for the given handle.
std::string Session::toBlockName(const EntityHandle& handle) const
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
  case EXO_MODEL:    return std::string(); break;
  case EXO_BLOCK:    blockId = 0; break;
  case EXO_SIDE_SET: blockId = 4; break;
  case EXO_NODE_SET: blockId = 7; break;
  default:
    return std::string();
    }
  vtkMultiBlockDataSet* typeSet =
    vtkMultiBlockDataSet::SafeDownCast(
      this->m_models[handle.modelNumber]->GetBlock(blockId));
  if (!typeSet || handle.entityId >= typeSet->GetNumberOfBlocks())
    return std::string();
  return std::string(
    typeSet->GetMetaData(handle.entityId)->Get(
      vtkCompositeDataSet::NAME()));
}

// A method that helps convert vtkPolyData into an SMTK Tessellation.
static void AddCellsToTessellation(
  vtkPoints* pts,
  vtkCellArray* cells,
  smtkCellTessRole role,
  std::map<vtkIdType,int>& vertMap,
  smtk::model::Tessellation& tess)
{
  vtkIdType npts;
  vtkIdType* conn;
  std::vector<int> tconn;
  std::map<vtkIdType,int>::iterator pit;
  for (cells->InitTraversal(); cells->GetNextCell(npts, conn); )
    {
    tconn.clear();
    tconn.reserve(npts + 2);
    switch (role)
      {
    case SMTK_ROLE_VERTS:
      if (npts > 1)
        {
        tconn.push_back(TESS_POLYVERTEX);
        tconn.push_back(npts);
        }
      else
        {
        tconn.push_back(TESS_VERTEX);
        }
      break;
    case SMTK_ROLE_LINES:
      tconn.push_back(TESS_POLYLINE);
      tconn.push_back(npts);
      break;
    case SMTK_ROLE_POLYS:
      switch (npts)
        {
      case 0:
      case 1:
      case 2:
        std::cerr
          << "Too few points (" << npts
          << ") for a surface primitive. Skipping.\n";
        continue;
        break;
      case 3: tconn.push_back(TESS_TRIANGLE); break;
      case 4: tconn.push_back(TESS_QUAD); break;
      default: tconn.push_back(TESS_POLYGON); tconn.push_back(npts); break;
        }
      break;
    default:
      std::cerr << "Unknown tessellation role " << role << ". Skipping.\n";
      continue;
      break;
      }
    for (vtkIdType i = 0; i < npts; ++i)
      {
      if ((pit = vertMap.find(conn[i])) == vertMap.end())
        pit = vertMap.insert(
          std::pair<vtkIdType,int>(
            conn[i], tess.addCoords(pts->GetPoint(conn[i])))).first;
      tconn.push_back(pit->second );
      }
    tess.insertNextCell(tconn);
    }
}


bool Session::addTessellation(
  const smtk::model::EntityRef& entityref,
  const EntityHandle& handle)
{
  if (entityref.hasTessellation())
    return true; // no need to recompute.

  vtkDataObject* data = this->toBlock<vtkDataObject>(handle);
  if (!data)
    return false; // can't squeeze triangles from a NULL

  vtkNew<vtkGeometryFilter> bdyFilter;
  bdyFilter->MergingOff();
  bdyFilter->SetInputDataObject(data);
  bdyFilter->Update();
  vtkPolyData* bdy = bdyFilter->GetOutput();

  if (!bdy)
    return SESSION_NOTHING;

  smtk::model::Tessellation tess;
  std::map<vtkIdType,int> vertMap;
  vtkPoints* pts = bdy->GetPoints();
  AddCellsToTessellation(pts, bdy->GetVerts(), SMTK_ROLE_VERTS, vertMap, tess);
  AddCellsToTessellation(pts, bdy->GetLines(), SMTK_ROLE_LINES, vertMap, tess);
  AddCellsToTessellation(pts, bdy->GetPolys(), SMTK_ROLE_POLYS, vertMap, tess);
  if (bdy->GetStrips() && bdy->GetStrips()->GetNumberOfCells() > 0)
    {
    std::cerr << "Warning: Triangle strips in discrete cells are unsupported. Ignoring.\n";
    }
  if (!vertMap.empty())
    entityref.manager()->setTessellation(entityref.entity(), tess);

  return true;
}

    } // namespace exodus
  } // namespace bridge
} // namespace smtk

// ++ 1 ++
#include "Session_json.h"

smtkImplementsModelingKernel(
  exodus,
  Session_json,
  SessionHasNoStaticSetup,
  smtk::bridge::exodus::Session,
  true /* inherit "universal" operators */
);
// -- 1 --
