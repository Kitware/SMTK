//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/discrete/Bridge.h"

#include "smtk/common/UUID.h"
#include "smtk/AutoInit.h"

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
#include "smtk/model/Manager.h"
#include "smtk/model/Shell.h"
#include "smtk/model/Chain.h"
#include "smtk/model/Loop.h"

#include "vtkCMBModelReadOperator.h"
#include "vtkCMBModelWriterV5.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelGeometricEntity.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkModel.h"
#include "vtkModelMaterial.h"
#include "vtkModelEdge.h"
#include "vtkModelEdgeUse.h"
#include "vtkModelEntity.h"
#include "vtkModelFace.h"
#include "vtkModelFaceUse.h"
#include "vtkModelGeometricEntity.h"
#include "vtkModelItemIterator.h"
#include "vtkModelLoopUse.h"
#include "vtkModelRegion.h"
#include "vtkModelShellUse.h"
#include "vtkModelUserName.h"
#include "vtkModelVertex.h"
#include "vtkModelVertexUse.h"

#include "vtkCommand.h"
#include "vtkCellArray.h"
#include "vtkCompositeDataPipeline.h" // for UPDATE_COMPOSITE_INDICES()
#include "vtkInformation.h"
#include "vtkInformationIntegerVectorKey.h"
#include "vtkNew.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkUnsignedIntArray.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {
    namespace discrete {

/// Track which models are tracked by which bridges.
std::map<vtkDiscreteModel*,Bridge::WeakPtr> Bridge::s_modelsToBridges;
/// Associate UUIDs to vtkDiscreteModelWrapper instances.
std::map<smtk::common::UUID,vtkSmartPointer<vtkDiscreteModelWrapper> > Bridge::s_modelIdsToRefs;
/// Associate vtkDiscreteModelWrapper instances to UUIDs.
std::map<vtkSmartPointer<vtkDiscreteModelWrapper>, smtk::common::UUID> Bridge::s_modelRefsToIds;

// Local helper to uniquely determine an integer "sense" for pairs of edge uses of an edge.
// FIXME: This really should assign the integer so that the pairs form a single cycle.
//        For example, consider edge uses a, b, c, d, e, f, g, h that are paired
//        like so: (a,b), (c,d), (e,h), (f,g) with the further constraint that
//        CCW(a) = c, CCW(d) = f, CCW(g) = h, CCW(e) = b
//        CCW(b) = a, CCW(c) = d, CCW(h) = e, CCW(g) = f.
//        In this case, [(a,b), (c,d), (e,h), (f,g)] should be assigned senses
//        [0, 1, 3, 2] or [1, 3, 2, 0], [3, 2, 0, 1], or [2, 0, 1, 3].
//        This way, the "next" sense obtained by incrementing (decrementing)
//        a sense moves CCW (CW) around the edge.
//        But I do not see how CMB provides information about CCW/CW pairings.
static int senseOfEdgeUse(vtkModelEdgeUse* srcEdgeUse)
{
  if (!srcEdgeUse) return -1;
  vtkModelEdge* refEdge = srcEdgeUse->GetModelEdge();
  if (!refEdge) return -1;
  std::map<vtkModelEdgeUse*,int> senses;
  int ns = 0;
  int neu = refEdge->GetNumberOfModelEdgeUses();
  for (int i = 0; i < neu; ++i)
    {
    vtkModelEdgeUse* eu = refEdge->GetModelEdgeUse(i);
    vtkModelEdgeUse* eup = eu->GetPairedModelEdgeUse();
    if (senses.find(eu) == senses.end())
      {
      senses[eu] = ns;
      senses[eup] = ns;
      ++ns;
      }
    if (eu == srcEdgeUse || eup == srcEdgeUse)
      {
      //std::cout << "sense(" << srcEdgeUse << ") = " << senses[srcEdgeUse] << "\n";
      return senses[eu];
      }
    }
  //std::cout << "sense(" << srcEdgeUse << ") = " << -1 << "\n";
  return -1; // The parent edge does not list srcEdgeUse.
}

class vtkItemWatcherCommand : public vtkCommand
{
public:
  static vtkItemWatcherCommand* New() { return new vtkItemWatcherCommand; }
  vtkTypeMacro(vtkItemWatcherCommand,vtkCommand);
  virtual void Execute(
    vtkObject* caller, unsigned long eventId, void* callData)
    {
    (void)eventId;
    smtk::common::UUID uid = bridge->findOrSetEntityUUID(vtkInformation::SafeDownCast(caller));
    vtkModelItem* item = bridge->entityForUUID(uid);
    std::cout << "Item " << item << " deleted. Was " << uid << "\n";
    }

  Bridge* bridge;
};

/**\brief Default constructor.
  *
  */
Bridge::Bridge()
{
  this->initializeOperatorSystem(Bridge::s_operators);
  this->m_itemWatcher = vtkItemWatcherCommand::New();
  this->m_itemWatcher->bridge = this;
}

/// Public virtual destructor required by base class.
Bridge::~Bridge()
{
  this->m_itemWatcher->Delete();
}

/// The CGM bridge supports smtk::model::BRIDGE_EVERYTHING.
smtk::model::BridgedInfoBits Bridge::allSupportedInformation() const
{
  return smtk::model::BRIDGE_EVERYTHING;
}

/**\brief Create records in \a manager that reflect the CMB \a entity.
  *
  */
smtk::model::Cursor Bridge::addCMBEntityToManager(
  const smtk::common::UUID& uid,
  smtk::model::ManagerPtr manager,
  int relDepth)
{
  vtkModelItem* ent = this->entityForUUID(uid);
  if (ent)
    return this->addCMBEntityToManager(uid, ent, manager, relDepth);
  return smtk::model::Cursor();
}

void Bridge::assignUUIDs(const std::vector<vtkModelItem*>& ents, vtkAbstractArray* arr)
{
  vtkUnsignedIntArray* vuid = vtkUnsignedIntArray::SafeDownCast(arr);
  std::vector<vtkModelItem*>::const_iterator it;
  if (arr)
    { // Assign UUIDs to entities from the input array
    if (
      vuid &&
      vuid->GetNumberOfComponents() * sizeof(unsigned int) == smtk::common::UUID::size() &&
      vuid->GetNumberOfTuples() == static_cast<vtkIdType>(ents.size()))
      {
      smtk::common::UUID::const_iterator raw =
        reinterpret_cast<smtk::common::UUID::const_iterator>(vuid->GetPointer(0));
      int i;
      for (i = 0, it = ents.begin(); it != ents.end(); ++it, i += smtk::common::UUID::size())
        {
        smtk::common::UUID eid(raw + i, raw + i + smtk::common::UUID::size());
        if (!eid.isNull())
          this->assignUUIDToEntity(eid, *it);
        //std::cout << "item " << *it << " (" << (*it)->GetClassName() << ")" << " UUID " << eid << " " << (i/16) << "\n";
        }
      }
    else
      {
      std::cerr
        << "Error: ID array " << arr << " (" << arr->GetClassName()
        << ", " << arr->GetNumberOfComponents() << " comps"
        << ", " << arr->GetNumberOfTuples() << " tuples"
        << ")" << " is not a vtkUnsignedIntArray or has wrong size.\n";
      }
    }
  else
    { // Generate new UUIDs and assign to entities.
    int i = 0;
    for (it = ents.begin(); it != ents.end(); ++it, ++i)
      {
      smtk::common::UUID entId =
        this->findOrSetEntityUUID(*it);
      (void)entId;
      //std::cout << "item " << *it << " (" << (*it)->GetClassName() << ")" << " UUID " << entId << " " << i << "\n";
      }
    }
}

vtkUnsignedIntArray* Bridge::retrieveUUIDs(
    vtkDiscreteModel* model, const std::vector<vtkModelItem*>& ents)
{
  std::map<vtkDiscreteModel*,WeakPtr>::iterator bridgeIt =
    Bridge::s_modelsToBridges.find(model);
  if (bridgeIt == Bridge::s_modelsToBridges.end())
    return NULL;
  Ptr bridge = bridgeIt->second.lock();
  if (!bridge)
    return NULL;

  vtkUnsignedIntArray* vuid = vtkUnsignedIntArray::New();
  vuid->SetNumberOfComponents(smtk::common::UUID::size() / sizeof(unsigned int));
  vuid->SetNumberOfTuples(static_cast<vtkIdType>(ents.size()));
  int* ptr = reinterpret_cast<int*>(vuid->GetPointer(0));
  smtk::common::UUID::iterator uptr =
    reinterpret_cast<smtk::common::UUID::iterator>(ptr);
  std::vector<vtkModelItem*>::const_iterator it;
  for (it = ents.begin(); it != ents.end(); ++it)
    {
    smtk::common::UUID eid = bridge->findOrSetEntityUUID(*it);
    smtk::common::UUID::iterator eidit;
    for (eidit = eid.begin(); eidit != eid.end(); ++eidit, ++uptr)
      *uptr = *eidit;
    }
  return vuid;
}

smtk::common::UUID Bridge::ImportEntitiesFromFileNameIntoManager(
  const std::string& filename,
  const std::string& filetype,
  smtk::model::ManagerPtr manager)
{
  // Make sure that the bridge has a list of operators it can provide.

  // TODO: Register an attribute type for UUID on CMB's models?
  if (filetype != "cmb")
    {
    std::cerr << "Unsupported file type \"" << filetype << "\" (not \"cmb\").\n";
    return smtk::common::UUID::null();
    }

  vtkNew<vtkDiscreteModelWrapper> mod;
  vtkNew<vtkCMBModelReadOperator> rdr;
  rdr->SetFileName(filename.c_str());
  rdr->Operate(mod.GetPointer());
  if (!rdr->GetOperateSucceeded())
    {
    std::cerr << "Could not read file \"" << filename << "\".\n";
    return smtk::common::UUID::null();
    }
  return this->trackModel(mod.GetPointer(), filename, manager);
}

int Bridge::ExportEntitiesToFileOfNameAndType(
  const std::vector<smtk::model::Cursor>& entities,
  const std::string& filename,
  const std::string& filetype)
{
  if (filetype != "cmb")
    {
    std::cerr << "File type must be \"cmb\", not \"" << filetype << "\"\n";
    return 1;
    }

  std::set<vtkDiscreteModel*> refsOut;
  smtk::model::CursorArray::const_iterator it;
  for (it = entities.begin(); it != entities.end(); ++it)
    {
    // TODO: obtain from it->modelOwningEntity() ... don't assume *it is a model.
    vtkModelItem* ent = this->entityForUUID(it->entity());
    vtkDiscreteModel* model = dynamic_cast<vtkDiscreteModel*>(ent);
    if (model)
      refsOut.insert(model);
    }
  if (refsOut.size() <= 0)
    return 1;

  vtkNew<vtkCMBModelWriterV5> wri;
  wri->SetDataModeToAscii(); // for debugging only
  std::set<vtkDiscreteModel*>::size_type nfiles = refsOut.size();
  int i = 0;
  for (std::set<vtkDiscreteModel*>::iterator mit = refsOut.begin(); mit != refsOut.end(); ++mit, ++i)
    {
    std::ostringstream fname;
    fname << filename;
    if (nfiles > 1)
      fname << "_" << i;
    wri->SetFileName(fname.str().c_str());
    wri->Write(*mit);
    std::cout << "Wrote " << fname.str() << "\n";
    }

  return 0;
}

vtkDiscreteModelWrapper* Bridge::findModel(const smtk::common::UUID& uid) const
{
  std::map<smtk::common::UUID,vtkSmartPointer<vtkDiscreteModelWrapper> >::const_iterator it;
  if ((it = Bridge::s_modelIdsToRefs.find(uid)) != Bridge::s_modelIdsToRefs.end())
    return it->second.GetPointer();
  return NULL;
}

/**\brief Populate records for \a cursor that reflect the CGM \a entity.
  *
  */
smtk::model::BridgedInfoBits Bridge::transcribeInternal(
  const smtk::model::Cursor& entity, BridgedInfoBits requestedInfo)
{
  (void)entity;
  (void)requestedInfo;
  return 0;
}

smtk::common::UUID Bridge::trackModel(
  vtkDiscreteModelWrapper* mod, const std::string& url,
  smtk::model::ManagerPtr manager)
{
  vtkDiscreteModel* dmod = mod->GetModel();
  if (!dmod)
    return smtk::common::UUID::null();

  // Add or obtain the model's UUID
  smtk::common::UUID mid = this->findOrSetEntityUUID(dmod);
  // Track the model (keep a strong reference to it)
  // by UUID as well as the inverse map for quick reference:
  Bridge::s_modelIdsToRefs[mid] = mod;
  Bridge::s_modelRefsToIds[mod] = mid;
  this->m_itemsToRefs[mid] = dmod;
  Bridge::s_modelsToBridges[dmod] = shared_from_this();
  manager->setBridgeForModel(shared_from_this(), mid);

  // Now add the record to manager and assign the URL to
  // the model as a string property.
  smtk::model::Cursor c = this->addCMBEntityToManager(mid, dmod, manager, 4);
  c.setStringProperty("url", url);

  return mid;
}

bool Bridge::assignUUIDToEntity(
  const smtk::common::UUID& itemId, vtkModelItem* item)
{
  if (!item || itemId.isNull())
    return false;
  vtkInformation* mp = item->GetProperties();
  mp->AddObserver(vtkCommand::DeleteEvent, this->m_itemWatcher);
  mp->Set(vtkCompositeDataPipeline::UPDATE_COMPOSITE_INDICES(),
    const_cast<int*>(reinterpret_cast<const int*>(itemId.begin())),
    16/sizeof(unsigned int));
  this->m_itemsToRefs[itemId] = item;
  return true;
}

smtk::common::UUID Bridge::findOrSetEntityUUID(vtkModelItem* item)
{
  vtkInformation* mp = item->GetProperties();
  return this->findOrSetEntityUUID(mp);
}

smtk::common::UUID Bridge::findOrSetEntityUUID(vtkInformation* mp)
{
  smtk::common::UUID mid;
  // Use UPDATE_COMPOSITE_INDICES() to hold a UUID.
  if (mp->Has(vtkCompositeDataPipeline::UPDATE_COMPOSITE_INDICES()))
    {
    int rawUUID[16/sizeof(int)];
    mp->Get(vtkCompositeDataPipeline::UPDATE_COMPOSITE_INDICES(), rawUUID);
    smtk::common::UUID::const_iterator rawAddr =
      reinterpret_cast<const smtk::common::UUID::iterator>(&rawUUID[0]);
    mid = smtk::common::UUID(rawAddr, rawAddr + smtk::common::UUID::size());
    }
  else
    {
    mid = this->m_idGenerator.random();
    mp->Set(vtkCompositeDataPipeline::UPDATE_COMPOSITE_INDICES(),
      reinterpret_cast<int*>(mid.begin()),
      16/sizeof(unsigned int));
    }
  return mid;
}

// vtkDiscreteModel* Bridge::owningModel(vtkModelItem* e);

/// Obtain the mapping from a UUID to a model entity.
vtkModelItem* Bridge::entityForUUID(const smtk::common::UUID& uid)
{
  if (uid.isNull())
    return NULL;

  std::map<smtk::common::UUID,vtkModelItem*>::const_iterator iref =
    this->m_itemsToRefs.find(uid);
  if (iref == this->m_itemsToRefs.end())
    return NULL;

  return iref->second;
}

smtk::model::Cursor Bridge::addCMBEntityToManager(
  const smtk::common::UUID& uid, vtkModelItem* ent, smtk::model::ManagerPtr manager, int relDepth)
{
  this->assignUUIDToEntity(uid, ent);
  vtkModel* modelEntity = dynamic_cast<vtkModel*>(ent);
  vtkModelGeometricEntity* cellEntity = dynamic_cast<vtkModelGeometricEntity*>(ent);
  vtkModelEntity* otherEntity = dynamic_cast<vtkModelEntity*>(ent);
  if (modelEntity)
    {
    return this->addBodyToManager(uid, modelEntity, manager, relDepth);
    }
  else if (cellEntity)
    {
    vtkModelRegion* region = dynamic_cast<vtkModelRegion*>(otherEntity);
    vtkModelFace* face = dynamic_cast<vtkModelFace*>(otherEntity);
    vtkModelEdge* edge = dynamic_cast<vtkModelEdge*>(otherEntity);
    vtkModelVertex* vert = dynamic_cast<vtkModelVertex*>(otherEntity);
    if (region) return this->addVolumeToManager(uid, region, manager, relDepth);
    else if (face) return this->addFaceToManager(uid, face, manager, relDepth);
    else if (edge) return this->addEdgeToManager(uid, edge, manager, relDepth);
    else if (vert) return this->addVertexToManager(uid, vert, manager, relDepth);
    else
      {
      std::cerr
        << "Unknown vtkModelGeometricEntity subclass \""
        << cellEntity->GetClassName() << "\" encountered. Ignoring.\n";
      }
    }
  else if (otherEntity)
    {
    vtkModelFaceUse* faceUse = dynamic_cast<vtkModelFaceUse*>(otherEntity);
    vtkModelEdgeUse* edgeUse = dynamic_cast<vtkModelEdgeUse*>(otherEntity);
    vtkModelVertexUse* vertUse = dynamic_cast<vtkModelVertexUse*>(otherEntity);
    vtkDiscreteModelEntityGroup* group = dynamic_cast<vtkDiscreteModelEntityGroup*>(otherEntity);
    vtkModelMaterial* material = dynamic_cast<vtkModelMaterial*>(otherEntity);
    vtkModelShellUse* shell = dynamic_cast<vtkModelShellUse*>(otherEntity);
    vtkModelLoopUse* loop = dynamic_cast<vtkModelLoopUse*>(otherEntity);
    if (faceUse) return this->addFaceUseToManager(uid, faceUse, manager, relDepth);
    else if (edgeUse) return this->addEdgeUseToManager(uid, edgeUse, manager, relDepth);
    else if (vertUse) return this->addVertexUseToManager(uid, vertUse, manager, relDepth);
    else if (group) return this->addGroupToManager(uid, group, manager, relDepth);
    else if (material) return this->addMaterialToManager(uid, material, manager, relDepth);
    else if (shell) return this->addShellToManager(uid, shell, manager, relDepth);
    else if (loop) return this->addLoopToManager(uid, loop, manager, relDepth);
    else
      {
      std::cerr
        << "Unknown vtkModelEntity subclass \""
        << otherEntity->GetClassName() << "\" encountered. Ignoring.\n";
      }
    }
  return smtk::model::Cursor();
}

// This is a mind-altering template class that exists to
// allow a single function to iterate over a collection
// and invoke a different method (M) on a different class
// (P) for each contained object (C).
// Specifically, a lot of the smtk::model::Cursor subclasses
// provide methods for adding/modifying relationships.
template<class P, class C, P& (P::*M)(const C&)>
class CursorHelper
{
public:
  CursorHelper()
    {
    }

  typedef C ChildType;

  void invoke(P& parent, const C& child) const
    {
    (parent.*M)(child);
    }
};

typedef CursorHelper<
  smtk::model::ModelEntity,
  smtk::model::CellEntity,
  &smtk::model::ModelEntity::addCell
> AddCellToModelHelper;

typedef CursorHelper<
  smtk::model::ModelEntity,
  smtk::model::GroupEntity,
  &smtk::model::ModelEntity::addGroup
> AddGroupToModelHelper;

typedef CursorHelper<
  smtk::model::GroupEntity,
  smtk::model::Cursor,
  &smtk::model::GroupEntity::addEntity
> AddEntityToGroupHelper;

typedef CursorHelper<
  smtk::model::Volume,
  smtk::model::VolumeUse,
  &smtk::model::Volume::setVolumeUse
> AddVolumeUseToVolumeHelper;

typedef CursorHelper<
  smtk::model::Cursor,
  smtk::model::Cursor,
  &smtk::model::Cursor::addRawRelation
> AddRawRelationHelper;

/// Internal only. Add entities from \a it to \a parent using \a helper.
template<class P, typename H>
void Bridge::addEntities(P& parent, vtkModelItemIterator* it, const H& helper, int relDepth)
{
  for (it->Begin(); !it->IsAtEnd(); it->Next())
    {
    typename H::ChildType child =
      this->addCMBEntityToManager(
        this->findOrSetEntityUUID(
          it->GetCurrentItem()),
        it->GetCurrentItem(), parent.manager(), relDepth);
    helper.invoke(parent, child);
    }
  it->Delete();
}

/// Internal only. Add entities from a \a childContainer to \a parent using \a helper.
template<class P, typename C, typename H>
void Bridge::addEntityArray(P& parent, C& childContainer, const H& helper, int relDepth)
{
  typename C::const_iterator cit;
  for (cit = childContainer.begin(); cit != childContainer.end(); ++cit)
    {
    typename H::ChildType child =
      this->addCMBEntityToManager(
        this->findOrSetEntityUUID(*cit),
        *cit, parent.manager(), relDepth);
    helper.invoke(parent, child);
    }
}

// A method that helps convert vtkPolyData into an SMTK Tessellation.
static void AddSimplicesToTess(
  vtkPoints* pts,
  vtkCellArray* cells,
  std::map<vtkIdType,int>& vertMap,
  smtk::model::Tessellation& tess)
{
  vtkIdType npts;
  vtkIdType* conn;
  std::vector<int> tconn;
  std::map<vtkIdType,int>::iterator pit;
  for (cells->InitTraversal(); cells->GetNextCell(npts, conn); )
    {
    tconn.resize(npts);
    for (vtkIdType i = 0; i < npts; ++i)
      {
      if ((pit = vertMap.find(conn[i])) == vertMap.end())
        pit = vertMap.insert(
          std::pair<vtkIdType,int>(
            conn[i], tess.addCoords(pts->GetPoint(conn[i])))).first;
      tconn[i] = pit->second;
      }
    switch (npts)
      {
    case 1: tess.addPoint(tconn[0]); break;
    case 2: tess.addLine(tconn[0], tconn[1]); break;
    case 3: tess.addTriangle(tconn[0], tconn[1], tconn[2]); break;
    default: std::cerr << "Unhandled polydata primitive " << npts << " pts\n"; break;
      }
    }
}

/**\brief Obtain the tessellation of \a cellIn and add it to \a cellOut.
  */
bool Bridge::addTessellation(const smtk::model::Cursor& cellOut, vtkModelGeometricEntity* cellIn)
{
  bool hasTess = false;
  vtkPolyData* poly;
  if (cellIn && (poly = vtkPolyData::SafeDownCast(cellIn->GetGeometry())))
    {
    smtk::model::Tessellation tess;
    std::map<vtkIdType,int> vertMap;
    vtkPoints* pts = poly->GetPoints();
    vtkCellArray* cells;
    cells = poly->GetVerts();
    AddSimplicesToTess(pts, poly->GetVerts(), vertMap, tess);
    AddSimplicesToTess(pts, poly->GetLines(), vertMap, tess);
    AddSimplicesToTess(pts, poly->GetPolys(), vertMap, tess);
    if (!vertMap.empty())
      cellOut.manager()->setTessellation(cellOut.entity(), tess);
    }
  return hasTess;
}

/**\brief Add float-, int-, and string-properties from the \a cellIn to \a cellOut.
  */
bool Bridge::addProperties(
  smtk::model::Cursor& cellOut,
  vtkModelItem* cellIn,
  smtk::model::BitFlags props)
{
  bool hasProps = true;
  vtkModelEntity* erec;
  if (cellIn && (erec = vtkModelEntity::SafeDownCast(cellIn)))
    {
    if (props & smtk::model::FLOAT_PROPERTY)
      {
      double rgba[4];
      erec->GetColor(rgba);
      if (rgba[0] >= 0)
        cellOut.setColor(smtk::model::FloatList(rgba, rgba + 4));

      vtkModelMaterial* material = vtkModelMaterial::SafeDownCast(erec);
      if (material)
        {
        double* wid = material->GetWarehouseId();
        cellOut.setFloatProperty("warehouse id", smtk::model::FloatList(wid, wid + 2));
        }
      }
    if (props & smtk::model::INTEGER_PROPERTY)
      {
      cellOut.setIntegerProperty("visibility", erec->GetVisibility());
      // TODO: Should probably remove "cmb id" before merging branch:
      cellOut.setIntegerProperty("cmb id", erec->GetUniquePersistentId());
      }
    if (props & smtk::model::STRING_PROPERTY)
      {
      const char* uname = vtkModelUserName::GetUserName(erec);
      if (uname)
        cellOut.setName(uname);
      }
    }
  return hasProps;
}

/// Given a CMB \a body tagged with \a uid, create a record in \a manager for it.
smtk::model::ModelEntity Bridge::addBodyToManager(
  const smtk::common::UUID& uid,
  vtkModel* body,
  smtk::model::ManagerPtr manager,
  int relDepth)
{
  if (body)
    {
    smtk::model::ModelEntity model;
    smtk::model::BridgedInfoBits translated;
    bool already;
    if ((already = manager->findEntity(uid, false) ? true : false) || relDepth < 0)
      {
      translated = already ? smtk::model::BRIDGE_ENTITY_ARRANGED : smtk::model::BRIDGE_NOTHING;
      model = smtk::model::ModelEntity(manager,uid);
      }
    else
      {
      translated = smtk::model::BRIDGE_ENTITY_RECORD;
      model = manager->insertModel(
        uid, body->GetModelDimension(), 3,
        "Discrete Model"); // TODO: Model name???
      }
    if (relDepth >= 0)
      {
      // Add free cells, submodels, and groups.

      // Only add the highest-dimensional cells present;
      // FIXME: How does CMB indicate that edges/vertices are "floating"
      if (body->GetNumberOfAssociations(vtkModelRegionType))
        { // Add regions to model
        this->addEntities(model, body->NewIterator(vtkModelRegionType), AddCellToModelHelper(), relDepth - 1);
        }
      else if(body->GetNumberOfAssociations(vtkModelFaceType))
        { // Add faces to model
        this->addEntities(model, body->NewIterator(vtkModelFaceType), AddCellToModelHelper(), relDepth - 1);
        }
      else if(body->GetNumberOfAssociations(vtkModelEdgeType))
        { // Add edges to model
        this->addEntities(model, body->NewIterator(vtkModelEdgeType), AddCellToModelHelper(), relDepth - 1);
        }
      else if(body->GetNumberOfAssociations(vtkModelVertexType))
        { // Add vertices to model
        this->addEntities(model, body->NewIterator(vtkModelVertexType), AddCellToModelHelper(), relDepth - 1);
        }

      // FIXME: How does CMB indicate groups are part of a model?
      if (body->GetNumberOfAssociations(vtkDiscreteModelEntityGroupType))
        { // Add regions to model
        this->addEntities(model, body->NewIterator(vtkDiscreteModelEntityGroupType), AddGroupToModelHelper(), relDepth - 1);
        }

      translated |= smtk::model::BRIDGE_ENTITY_ARRANGED;
      }

    this->addProperties(model, body);
    translated |= smtk::model::BRIDGE_PROPERTIES;

    this->declareDanglingEntity(model, translated);
    return model;
    }
  return smtk::model::ModelEntity();
}

/// Given a CMB \a group tagged with \a uid, create a record in \a manager for it.
smtk::model::GroupEntity Bridge::addGroupToManager(
  const smtk::common::UUID& uid,
  vtkDiscreteModelEntityGroup* group,
  smtk::model::ManagerPtr manager,
  int relDepth)
{
  if (group)
    {
    smtk::model::GroupEntity result;
    smtk::model::BridgedInfoBits translated;
    bool already;
    if ((already = manager->findEntity(uid, false) ? true : false) || relDepth < 0)
      {
      translated = already ? smtk::model::BRIDGE_ENTITY_ARRANGED : smtk::model::BRIDGE_NOTHING;
      result = smtk::model::GroupEntity(manager, uid);
      }
    else
      {
      translated = smtk::model::BRIDGE_ENTITY_ARRANGED;
      result = manager->insertGroup(uid);
      }
    if (relDepth >= 0)
      {
      this->addEntities(result, group->NewModelEntityIterator(), AddEntityToGroupHelper(), relDepth - 1);
      }

    this->addProperties(result, group);
    translated |= smtk::model::BRIDGE_PROPERTIES;

    this->declareDanglingEntity(result, translated);
    return result;
    }
  return smtk::model::GroupEntity();
}

/// Given a CMB \a material tagged with \a uid, create a record in \a manager for it.
smtk::model::GroupEntity Bridge::addMaterialToManager(
  const smtk::common::UUID& uid,
  vtkModelMaterial* material,
  smtk::model::ManagerPtr manager,
  int relDepth)
{
  if (material)
    {
    smtk::model::BridgedInfoBits translated;
    smtk::model::GroupEntity result;
    bool already;
    if ((already = manager->findEntity(uid, false) ? true : false) || relDepth < 0)
      {
      translated = already ? smtk::model::BRIDGE_EVERYTHING : smtk::model::BRIDGE_NOTHING;
      result = smtk::model::GroupEntity(manager, uid);
      }
    else
      {
      translated = smtk::model::BRIDGE_ENTITY_ARRANGED;
      result = manager->insertGroup(uid);
      }
    if (relDepth >= 0)
      {
      // Add material relations and arrangements
      translated |= smtk::model::BRIDGE_ENTITY_ARRANGED;
      }

    // Add material properties:
    this->addProperties(result, material);
    translated |= smtk::model::BRIDGE_PROPERTIES;

    this->declareDanglingEntity(result, translated);
    return result;
    }
  return smtk::model::GroupEntity();
}

/// Given a CMB \a coFace tagged with \a uid, create a record in \a manager for it.
smtk::model::FaceUse Bridge::addFaceUseToManager(
  const smtk::common::UUID& uid,
  vtkModelFaceUse* coFace,
  smtk::model::ManagerPtr manager,
  int relDepth)
{
  if (coFace)
    {
    smtk::model::FaceUse result;
    smtk::model::BridgedInfoBits translated;
    bool already;
    smtk::model::Face matchingFace(
      manager, this->findOrSetEntityUUID(coFace->GetModelFace()));
    if ((already = manager->findEntity(uid, false) ? true : false) || relDepth < 0)
      {
      translated = already ? smtk::model::BRIDGE_ENTITY_ARRANGED : smtk::model::BRIDGE_NOTHING;
      result = smtk::model::FaceUse(manager, uid);
      }
    else
      {
      translated = smtk::model::BRIDGE_ENTITY_ARRANGED;
      // vtkModelFaceUse does not provide any orientation/sense info,
      // so we check the face to find its orientation. Blech.
      std::cout << "Face Use " << uid << " face " << matchingFace << " sense 0 " << " orient "
        << (coFace->GetModelFace()->GetModelFaceUse(1) == coFace ? "+" : "-") << "\n";
      result = manager->setFaceUse(
        uid, matchingFace, 0,
        coFace->GetModelFace()->GetModelFaceUse(1) == coFace ?
        smtk::model::POSITIVE : smtk::model::NEGATIVE);
      }

    if (relDepth >= 0)
      {
      // Add coFace relations and arrangements.
      vtkModelShellUse* shellUse = coFace->GetModelShellUse();
      if (shellUse)
        {
        smtk::common::UUID shellId = this->findOrSetEntityUUID(shellUse);

        this->addCMBEntityToManager(matchingFace.entity(), coFace->GetModelFace(), manager, relDepth - 1);
        this->addCMBEntityToManager(shellId, shellUse, manager, relDepth - 1);
        vtkModelItemIterator* loopIt = coFace->NewLoopUseIterator();
        smtk::model::Loops loops;
        for (loopIt->Begin(); !loopIt->IsAtEnd(); loopIt->Next())
          {
          vtkModelItem* loop = loopIt->GetCurrentItem();
          smtk::common::UUID loopId = this->findOrSetEntityUUID(loop);
          this->addCMBEntityToManager(loopId, loop, manager, relDepth - 1);
          loops.push_back(smtk::model::Loop(manager, loopId));
          }
        loopIt->Delete();
        smtk::model::FaceUse faceUse(manager, uid);
        faceUse.addShellEntities(loops);
        faceUse.setBoundingShellEntity(smtk::model::ShellEntity(manager,shellId));
        }
      }

    this->addProperties(result, coFace);
    translated |= smtk::model::BRIDGE_PROPERTIES;

    this->declareDanglingEntity(result, translated);
    return result;
    }
  return smtk::model::FaceUse();
}

/// Given a CMB \a coEdge tagged with \a uid, create a record in \a manager for it.
smtk::model::EdgeUse Bridge::addEdgeUseToManager(
  const smtk::common::UUID& uid,
  vtkModelEdgeUse* coEdge,
  smtk::model::ManagerPtr manager,
  int relDepth)
{
  if (coEdge)
    {
    smtk::model::BridgedInfoBits translated = smtk::model::BRIDGE_NOTHING;
    bool already;
    smtk::model::EdgeUse result(manager, uid);
    if ((already = manager->findEntity(uid, false) ? true : false) || relDepth < 0)
      {
      translated = already ? smtk::model::BRIDGE_ENTITY_ARRANGED : smtk::model::BRIDGE_NOTHING;
      }
    else
      {
      smtk::model::Edge matchingEdge(
        manager, this->findOrSetEntityUUID(coEdge->GetModelEdge()));
      if (manager->findEntity(matchingEdge.entity(), false) != NULL)
        { // Force the addition of the parent edge to the model.
        this->addEdgeToManager(matchingEdge.entity(), coEdge->GetModelEdge(), manager, 0);
        }
      if (relDepth >= 0)
        {
        // Now create the edge use with the proper relation:
        manager->setEdgeUse(
          uid, matchingEdge, senseOfEdgeUse(coEdge),
          coEdge->GetDirection() ? smtk::model::POSITIVE : smtk::model::NEGATIVE);
        // Finally, create its loop.
        vtkModelLoopUse* loopUse = coEdge->GetModelLoopUse();
        std::cout << "Edge use " << uid << " loop use " << loopUse << "\n";
        if (loopUse)
          {
          smtk::common::UUID luid = this->findOrSetEntityUUID(loopUse);
          smtk::model::Loop lpu = this->addLoopToManager(luid, loopUse, manager, relDepth - 1);
          }

        translated |= smtk::model::BRIDGE_ENTITY_ARRANGED;
        }

      this->addProperties(result, coEdge);
      translated |= smtk::model::BRIDGE_PROPERTIES;

      }
    return result;
    }
  return smtk::model::EdgeUse();
}

/// Given a CMB \a coVertex tagged with \a uid, create a record in \a manager for it.
smtk::model::VertexUse Bridge::addVertexUseToManager(
  const smtk::common::UUID& uid,
  vtkModelVertexUse* coVertex,
  smtk::model::ManagerPtr manager,
  int relDepth)
{
  if (coVertex && !manager->findEntity(uid, false))
    {
    smtk::model::VertexUse result(manager, uid);
    smtk::model::BridgedInfoBits translated = smtk::model::BRIDGE_NOTHING;
    if (relDepth >= 0)
      {
      // Add coVertex relations and arrangements
      }

    this->addProperties(result, coVertex);
    translated |= smtk::model::BRIDGE_PROPERTIES;

    return result;
    }
  return smtk::model::VertexUse();
}

/// Given a CMB \a shell tagged with \a uid, create a record in \a manager for it.
smtk::model::Shell Bridge::addShellToManager(
  const smtk::common::UUID& uid,
  vtkModelShellUse* shell,
  smtk::model::ManagerPtr manager,
  int relDepth)
{
  if (shell && !manager->findEntity(uid, false))
    {
    smtk::model::Shell result(manager, uid);
    smtk::model::BridgedInfoBits translated = smtk::model::BRIDGE_NOTHING;
    if (relDepth >= 0)
      {
      // Add shell relations and arrangements
      }

    this->addProperties(result, shell);
    translated |= smtk::model::BRIDGE_PROPERTIES;

    return result;
    }
  return smtk::model::Shell();
}

static vtkModelFaceUse* locateLoopInFace(
  vtkModelLoopUse* refLoop, int& faceUseOrientation, vtkModelLoopUse*& refLoopParent)
{
  vtkModelFace* refFace = refLoop->GetModelFace();
  // Consider both orientations of the face:
  for (int i = 0; i < 2; ++i)
    {
    vtkModelFaceUse* refFaceUse = refFace->GetModelFaceUse(i);
    // Now iterate over all the loops in this use.
    vtkModelItemIterator* loopUseIt = refFaceUse->NewLoopUseIterator();
    vtkModelLoopUse* other;
    for (loopUseIt->Begin(); !loopUseIt->IsAtEnd(); loopUseIt->Next())
      {
      if ((other = dynamic_cast<vtkModelLoopUse*>(loopUseIt->GetCurrentItem())))
        {
        if (other == refLoop)
          { // Found our loop, prepare output variables/return value:
          vtkModelLoopUse* outerLoop = refFaceUse->GetOuterLoopUse();
          refLoopParent = (other == outerLoop) ? NULL : outerLoop;
          faceUseOrientation = i;
          return refFaceUse;
          }
        }
      }
    loopUseIt->Delete();
    }
  // If we still haven't found the loop in its own face, then insanity prevails!
  return NULL;
}

/// Given a CMB \a loop tagged with \a uid, create a record in \a manager for it.
smtk::model::Loop Bridge::addLoopToManager(
  const smtk::common::UUID& uid,
  vtkModelLoopUse* refLoop,
  smtk::model::ManagerPtr manager,
  int relDepth)
{
  vtkModelFace* refFace;
  if (refLoop && (refFace = refLoop->GetModelFace()) && !manager->findEntity(uid, false))
    {
    smtk::model::BridgedInfoBits translated = smtk::model::BRIDGE_NOTHING;
    // Insert the loop, which means inserting the face use and face regardless of relDepth,
    // because the loop's orientation and nesting must be arranged relative to them.
    smtk::common::UUID fid = this->findOrSetEntityUUID(refFace);
    this->addFaceToManager(fid, refFace, manager, relDepth - 1);
    // Find the face *use* this loop belongs to and transcribe it.
    // Note that loop may be the child of a face use OR another loop (which we must then transcribe).
    vtkModelLoopUse* refLoopParent = NULL;
    int faceUseOrientation = -1;
    vtkModelFaceUse* refFaceUse = locateLoopInFace(refLoop, faceUseOrientation, refLoopParent);
    if (!refFaceUse || faceUseOrientation < 0) return smtk::model::Loop();
    smtk::common::UUID fuid = this->findOrSetEntityUUID(refFaceUse);
    smtk::model::FaceUse faceUse = this->addFaceUseToManager(fid, refFaceUse, manager, 0);
    smtk::model::Loop loop;
    if (refLoopParent)
      {
      smtk::common::UUID pluid = this->findOrSetEntityUUID(refLoopParent);
      smtk::model::Loop parentLoop = this->addLoopToManager(pluid, refLoopParent, manager, 0);
      loop = manager->setLoop(uid, parentLoop);
      }
    else
      {
      loop = manager->setLoop(uid, faceUse);
      }
    if (relDepth >= 0)
      {
      // Add loop relations and arrangements
      // Add the edge uses to the loop.
      int neu = refLoop->GetNumberOfModelEdgeUses();
      for (int i = 0; i < neu; ++i)
        {
        vtkModelEdgeUse* eu = refLoop->GetModelEdgeUse(i);
        smtk::common::UUID euid = this->findOrSetEntityUUID(eu);
        this->addEdgeUseToManager(euid, eu, manager, relDepth - 1);
        }
      }

    this->addProperties(loop, refLoop);
    translated |= smtk::model::BRIDGE_PROPERTIES;

    return loop;
    }
  return smtk::model::Loop();
}

/// Given a CMB \a refVolume tagged with \a uid, create a record in \a manager for it.
smtk::model::Volume Bridge::addVolumeToManager(
  const smtk::common::UUID& uid,
  vtkModelRegion* refVolume,
  smtk::model::ManagerPtr manager,
  int relDepth)
{
  if (refVolume && !manager->findEntity(uid, false))
    {
    smtk::model::Volume result(manager->insertVolume(uid));
    smtk::model::BridgedInfoBits translated = smtk::model::BRIDGE_NOTHING;
    if (relDepth >= 0)
      {
      // Add refVolume relations and arrangements
      this->addEntities(result, refVolume->NewAdjacentModelFaceIterator(), AddRawRelationHelper(), relDepth - 1);
      this->addEntities(result, refVolume->NewIterator(vtkModelShellUseType), AddVolumeUseToVolumeHelper(), relDepth - 1);
      }

    this->addProperties(result, refVolume);
    translated |= smtk::model::BRIDGE_PROPERTIES;

    return result;
    }
  return smtk::model::Volume();
}

/// Given a CMB \a refFace tagged with \a uid, create a record in \a manager for it.
smtk::model::Face Bridge::addFaceToManager(
  const smtk::common::UUID& uid,
  vtkModelFace* refFace,
  smtk::model::ManagerPtr manager,
  int relDepth)
{
  if (refFace && !manager->findEntity(uid, false))
    {
    smtk::model::Face result(manager->insertFace(uid));
    smtk::model::BridgedInfoBits translated = smtk::model::BRIDGE_NOTHING;
    if (relDepth >= 0)
      { // Add refFace relations and arrangements
      // If face uses exist, add them to the bridge.
      vtkModelFaceUse* fu;
      bool haveFaceUse = false;
      for (int i = 0; i < 2; ++i)
        {
        fu = refFace->GetModelFaceUse(i); // 0 = negative, 1 = positive
        if (fu)
          {
          haveFaceUse = true;
          smtk::common::UUID fuid = this->findOrSetEntityUUID(fu);
          this->addFaceUseToManager(fuid, fu, manager, relDepth - 1);
          // Now, since we are the "higher" end of the relationship,
          // arrange the use wrt ourself:
          manager->findCreateOrReplaceCellUseOfSenseAndOrientation(
            uid, 0, i ? smtk::model::POSITIVE : smtk::model::NEGATIVE, fuid);
          }
        }

     if (!haveFaceUse)
        { // Add a reference to the volume(s) directly (with no relationship)
        int nvols = refFace->GetNumberOfModelRegions();
        for (int i = 0; i < nvols; ++i)
          {
          vtkModelRegion* vol = refFace->GetModelRegion(i);
          if (vol)
            {
            Volume v(manager, this->findOrSetEntityUUID(vol));
            result.addRawRelation(v);
            }
          }
        }
      std::vector<vtkModelEdge*> edges;
      refFace->GetModelEdges(edges);
      this->addEntityArray(result, edges, AddRawRelationHelper(), relDepth - 1);
      // Add geometry, if any.
      this->addTessellation(result, refFace);
      }

    this->addProperties(result, refFace);
    translated |= smtk::model::BRIDGE_PROPERTIES;

    return result;
    }
  return smtk::model::Face();
}

/// Given a CMB \a refEdge tagged with \a uid, create a record in \a manager for it.
smtk::model::Edge Bridge::addEdgeToManager(
  const smtk::common::UUID& uid,
  vtkModelEdge* refEdge,
  smtk::model::ManagerPtr manager,
  int relDepth)
{
  if (refEdge && !manager->findEntity(uid, false))
    {
    smtk::model::Edge result(manager->insertEdge(uid));
    smtk::model::BridgedInfoBits translated = smtk::model::BRIDGE_NOTHING;
    if (relDepth >= 0)
      {
      // Add refEdge relations and arrangements
      int neu = refEdge->GetNumberOfModelEdgeUses();
      for (int i = 0; i < neu; ++i)
        {
        vtkModelEdgeUse* eu = refEdge->GetModelEdgeUse(i);
        smtk::common::UUID euid = this->findOrSetEntityUUID(eu);
        this->addEdgeUseToManager(euid, eu, manager, relDepth - 1);
        }
      // Add geometry, if any.
      this->addTessellation(result, refEdge);
      }

    this->addProperties(result, refEdge);
    translated |= smtk::model::BRIDGE_PROPERTIES;

    return result;
    }
  return smtk::model::Edge(manager, uid);
}

/// Given a CMB \a refVertex tagged with \a uid, create a record in \a manager for it.
smtk::model::Vertex Bridge::addVertexToManager(
  const smtk::common::UUID& uid,
  vtkModelVertex* refVertex,
  smtk::model::ManagerPtr manager,
  int relDepth)
{
  if (refVertex && !manager->findEntity(uid, false))
    {
    smtk::model::Vertex result(manager->insertVertex(uid));
    smtk::model::BridgedInfoBits translated = smtk::model::BRIDGE_NOTHING;
    if (relDepth >= 0)
      {
      // Add refVertex relations and arrangements
      // Add geometry, if any.
      this->addTessellation(result, refVertex);
      }

    this->addProperties(result, refVertex);
    translated |= smtk::model::BRIDGE_PROPERTIES;

    return result;
    }
  return smtk::model::Vertex();
}

/**\brief Set pre-construction options.
  *
  * Returns a non-zero value when the option is accepted
  * and zero when \a optName is unrecognized or
  * \a optVal is unacceptable.
  */
int Bridge::staticSetup(const std::string& optName, const smtk::model::StringList& optVal)
{
  return 0;
}

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#include "smtk/bridge/discrete/Bridge_json.h"
smtkImplementsModelingKernel(
  discrete,
  Bridge_json,
  smtk::bridge::discrete::Bridge::staticSetup,
  smtk::bridge::discrete::Bridge);

// Force these operators to be registered whenever the bridge is used:
smtkComponentInitMacro(smtk_discrete_read_operator);
smtkComponentInitMacro(smtk_discrete_merge_operator);
smtkComponentInitMacro(smtk_discrete_split_face_operator);
smtkComponentInitMacro(smtk_discrete_create_edges_operator);
