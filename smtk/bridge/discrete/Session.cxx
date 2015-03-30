//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/discrete/Session.h"

#include "smtk/common/UUID.h"
#include "smtk/AutoInit.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
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

enum smtkCellTessRole {
  SMTK_ROLE_VERTS,
  SMTK_ROLE_LINES,
  SMTK_ROLE_POLYS
};

/// Track which models are tracked by which sessions.
std::map<vtkDiscreteModel*,Session::WeakPtr> Session::s_modelsToSessions;
/// Associate UUIDs to vtkDiscreteModelWrapper instances.
std::map<smtk::common::UUID,vtkSmartPointer<vtkDiscreteModelWrapper> > Session::s_modelIdsToRefs;
/// Associate vtkDiscreteModelWrapper instances to UUIDs.
std::map<vtkSmartPointer<vtkDiscreteModelWrapper>, smtk::common::UUID> Session::s_modelRefsToIds;

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
    (void)callData;
    smtk::common::UUID uid = session->findOrSetEntityUUID(vtkInformation::SafeDownCast(caller));
    vtkModelItem* item = session->entityForUUID(uid);
    (void)item;
    //std::cout << "Item " << item << " deleted. Was " << uid << "\n";
    session->untrackEntity(uid);
    }

  Session* session;
};

/**\brief Default constructor.
  *
  */
Session::Session()
{
  this->initializeOperatorSystem(Session::s_operators);
  this->m_itemWatcher = vtkItemWatcherCommand::New();
  this->m_itemWatcher->session = this;
}

/// Public virtual destructor required by base class.
Session::~Session()
{
  // Remove any observers on model items that this session added:
  std::map<smtk::common::UUID, vtkWeakPointer<vtkModelItem> >::iterator mit;
  for (mit = this->m_itemsToRefs.begin(); mit != this->m_itemsToRefs.end(); ++mit)
    {
    if(!mit->second)
      continue;
    vtkInformation* mp = mit->second->GetProperties();
    mp->RemoveObserver(this->m_itemWatcher);
    }
  // Remove any models that this session owned from s_modelsToSessions.
  std::map<vtkDiscreteModel*,Session::WeakPtr>::iterator mbit;
  for (mbit = Session::s_modelsToSessions.begin(); mbit != Session::s_modelsToSessions.end(); )
    {
    if (mbit->second.lock().get() == this)
      {
      smtk::common::UUID modelId = this->findOrSetEntityUUID(mbit->first);
      Session::s_modelsToSessions.erase(mbit++);
      vtkSmartPointer<vtkDiscreteModelWrapper> modelPtr = Session::s_modelIdsToRefs[modelId];
      Session::s_modelIdsToRefs.erase(modelId);
      Session::s_modelRefsToIds.erase(modelPtr);
      }
    else
      {
      ++mbit;
      }
    }
  this->m_itemWatcher->Delete();
}

/// The CGM session supports smtk::model::SESSION_EVERYTHING.
smtk::model::SessionInfoBits Session::allSupportedInformation() const
{
  return smtk::model::SESSION_EVERYTHING;
}

/**\brief Create records in \a mgr that reflect the CMB \a entity.
  *
  */
smtk::model::EntityRef Session::addCMBEntityToManager(
  const smtk::common::UUID& uid,
  smtk::model::ManagerPtr mgr,
  int relDepth)
{
  vtkModelItem* ent = this->entityForUUID(uid);
  if (ent)
    return this->addCMBEntityToManager(uid, ent, mgr, relDepth);
  return smtk::model::EntityRef();
}

void Session::assignUUIDs(const std::vector<vtkModelItem*>& ents, vtkAbstractArray* arr)
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

vtkUnsignedIntArray* Session::retrieveUUIDs(
    vtkDiscreteModel* model, const std::vector<vtkModelItem*>& ents)
{
  std::map<vtkDiscreteModel*,WeakPtr>::iterator sessionIt =
    Session::s_modelsToSessions.find(model);
  if (sessionIt == Session::s_modelsToSessions.end())
    return NULL;
  Ptr session = sessionIt->second.lock();
  if (!session)
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
    smtk::common::UUID eid = session->findOrSetEntityUUID(*it);
    smtk::common::UUID::iterator eidit;
    for (eidit = eid.begin(); eidit != eid.end(); ++eidit, ++uptr)
      *uptr = *eidit;
    }
  return vuid;
}

int Session::ExportEntitiesToFileOfNameAndType(
  const std::vector<smtk::model::EntityRef>& entities,
  const std::string& filename,
  const std::string& filetype)
{
  if (filetype != "cmb")
    {
    std::cerr << "File type must be \"cmb\", not \"" << filetype << "\"\n";
    return 1;
    }

  std::set<vtkDiscreteModel*> refsOut;
  smtk::model::EntityRefArray::const_iterator it;
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

vtkDiscreteModelWrapper* Session::findModelEntity(const smtk::common::UUID& uid) const
{
  std::map<smtk::common::UUID,vtkSmartPointer<vtkDiscreteModelWrapper> >::const_iterator it;
  if ((it = Session::s_modelIdsToRefs.find(uid)) != Session::s_modelIdsToRefs.end())
    return it->second.GetPointer();
  return NULL;
}

/**\brief Populate records for \a entityref that reflect the CGM \a entity.
  *
  */
smtk::model::SessionInfoBits Session::transcribeInternal(
  const smtk::model::EntityRef& entity, SessionInfoBits requestedInfo)
{
  (void)entity;
  (void)requestedInfo;
  return 0;
}

smtk::common::UUID Session::trackModel(
  vtkDiscreteModelWrapper* mod, const std::string& url,
  smtk::model::ManagerPtr mgr)
{
  vtkDiscreteModel* dmod = mod->GetModel();
  if (!dmod)
    return smtk::common::UUID::null();

  // Add or obtain the model's UUID
  smtk::common::UUID mid = this->findOrSetEntityUUID(dmod);
  // Track the model (keep a strong reference to it)
  // by UUID as well as the inverse map for quick reference:
  Session::s_modelIdsToRefs[mid] = mod;
  Session::s_modelRefsToIds[mod] = mid;
  this->m_itemsToRefs[mid] = dmod;
  Session::s_modelsToSessions[dmod] = shared_from_this();
  smtk::model::Model smtkModel(mgr, mid);
  smtkModel.setSession(
    smtk::model::SessionRef(
      mgr, this->sessionId()));

  // Now add the record to manager and assign the URL to
  // the model as a string property.
  smtk::model::EntityRef c = this->addCMBEntityToManager(mid, dmod, mgr, 8);
  c.setStringProperty("url", url);

  return mid;
}

bool Session::assignUUIDToEntity(
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

smtk::common::UUID Session::findOrSetEntityUUID(vtkModelItem* item)
{
  vtkInformation* mp = item->GetProperties();
  return this->findOrSetEntityUUID(mp);
}

smtk::common::UUID Session::findOrSetEntityUUID(vtkInformation* mp)
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

// vtkDiscreteModel* Session::owningModel(vtkModelItem* e);

/// Obtain the mapping from a UUID to a model entity.
vtkModelItem* Session::entityForUUID(const smtk::common::UUID& uid)
{
  if (uid.isNull())
    return NULL;

  std::map<smtk::common::UUID,vtkWeakPointer<vtkModelItem> >::const_iterator iref =
    this->m_itemsToRefs.find(uid);
  if (iref == this->m_itemsToRefs.end())
    return NULL;

  return iref->second;
}

/// Erase the mapping from a UUID to a model entity.
void Session::untrackEntity(const smtk::common::UUID& uid)
{
  this->m_itemsToRefs.erase(uid);
}

smtk::model::EntityRef Session::addCMBEntityToManager(
  const smtk::common::UUID& uid, vtkModelItem* ent, smtk::model::ManagerPtr mgr, int relDepth)
{
  this->assignUUIDToEntity(uid, ent);
  vtkModel* modelEntity = dynamic_cast<vtkModel*>(ent);
  vtkModelGeometricEntity* cellEntity = dynamic_cast<vtkModelGeometricEntity*>(ent);
  vtkModelEntity* otherEntity = dynamic_cast<vtkModelEntity*>(ent);
  if (modelEntity)
    {
    return this->addBodyToManager(uid, modelEntity, mgr, relDepth);
    }
  else if (cellEntity)
    {
    vtkModelRegion* region = dynamic_cast<vtkModelRegion*>(otherEntity);
    vtkModelFace* face = dynamic_cast<vtkModelFace*>(otherEntity);
    vtkModelEdge* edge = dynamic_cast<vtkModelEdge*>(otherEntity);
    vtkModelVertex* vert = dynamic_cast<vtkModelVertex*>(otherEntity);
    if (region) return this->addVolumeToManager(uid, region, mgr, relDepth);
    else if (face) return this->addFaceToManager(uid, face, mgr, relDepth);
    else if (edge) return this->addEdgeToManager(uid, edge, mgr, relDepth);
    else if (vert) return this->addVertexToManager(uid, vert, mgr, relDepth);
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
    if (faceUse) return this->addFaceUseToManager(uid, faceUse, mgr, relDepth);
    else if (edgeUse) return this->addEdgeUseToManager(uid, edgeUse, mgr, relDepth);
    else if (vertUse) return this->addVertexUseToManager(uid, vertUse, mgr, relDepth);
    else if (group) return this->addGroupToManager(uid, group, mgr, relDepth);
    else if (material) return this->addMaterialToManager(uid, material, mgr, relDepth);
    else if (shell) return this->addShellToManager(uid, shell, mgr, relDepth);
    else if (loop) return this->addLoopToManager(uid, loop, mgr, relDepth);
    else
      {
      std::cerr
        << "Unknown vtkModel subclass \""
        << otherEntity->GetClassName() << "\" encountered. Ignoring.\n";
      }
    }
  return smtk::model::EntityRef();
}

// This is a mind-altering template class that exists to
// allow a single function to iterate over a collection
// and invoke a different method (M) on a different class
// (P) for each contained object (C).
// Specifically, a lot of the smtk::model::EntityRef subclasses
// provide methods for adding/modifying relationships.
template<class P, class C, P& (P::*M)(const C&)>
class EntityRefHelper
{
public:
  EntityRefHelper()
    {
    }

  typedef C ChildType;

  void invoke(P& parent, const C& child) const
    {
    (parent.*M)(child);
    }
};

typedef EntityRefHelper<
  smtk::model::Model,
  smtk::model::CellEntity,
  &smtk::model::Model::addCell
> AddCellToModelHelper;

typedef EntityRefHelper<
  smtk::model::Model,
  smtk::model::Group,
  &smtk::model::Model::addGroup
> AddGroupToModelHelper;

typedef EntityRefHelper<
  smtk::model::Group,
  smtk::model::EntityRef,
  &smtk::model::Group::addEntity
> AddEntityToGroupHelper;

typedef EntityRefHelper<
  smtk::model::Volume,
  smtk::model::VolumeUse,
  &smtk::model::Volume::setVolumeUse
> AddVolumeUseToVolumeHelper;

typedef EntityRefHelper<
  smtk::model::EntityRef,
  smtk::model::EntityRef,
  &smtk::model::EntityRef::addRawRelation
> AddRawRelationHelper;

/// Internal only. Add entities from \a it to \a parent using \a helper.
template<class P, typename H>
void Session::addEntities(P& parent, vtkModelItemIterator* it, const H& helper, int relDepth)
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
void Session::addEntityArray(P& parent, C& childContainer, const H& helper, int relDepth)
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

/**\brief Obtain the tessellation of \a cellIn and add it to \a cellOut.
  */
bool Session::addTessellation(const smtk::model::EntityRef& cellOut, vtkModelGeometricEntity* cellIn)
{
  bool hasTess = false;
  vtkPolyData* poly;
  if (cellIn && (poly = vtkPolyData::SafeDownCast(cellIn->GetGeometry())))
    {
    smtk::model::Tessellation tess;
    std::map<vtkIdType,int> vertMap;
    vtkPoints* pts = poly->GetPoints();
    AddCellsToTessellation(pts, poly->GetVerts(), SMTK_ROLE_VERTS, vertMap, tess);
    AddCellsToTessellation(pts, poly->GetLines(), SMTK_ROLE_LINES, vertMap, tess);
    AddCellsToTessellation(pts, poly->GetPolys(), SMTK_ROLE_POLYS, vertMap, tess);
    if (poly->GetStrips() && poly->GetStrips()->GetNumberOfCells() > 0)
      {
      std::cerr << "Warning: Triangle strips in discrete cells are unsupported. Ignoring.\n";
      }
    if (!vertMap.empty())
      cellOut.manager()->setTessellation(cellOut.entity(), tess);
    }
  return hasTess;
}

/**\brief Add float-, int-, and string-properties from the \a cellIn to \a cellOut.
  */
bool Session::addProperties(
  smtk::model::EntityRef& cellOut,
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
smtk::model::Model Session::addBodyToManager(
  const smtk::common::UUID& uid,
  vtkModel* body,
  smtk::model::ManagerPtr mgr,
  int relDepth)
{
  if (body)
    {
    smtk::model::Model model;
    smtk::model::SessionInfoBits translated;
    bool already;
    if ((already = mgr->findEntity(uid, false) ? true : false) || relDepth < 0)
      {
      translated = already ? smtk::model::SESSION_ENTITY_ARRANGED : smtk::model::SESSION_NOTHING;
      model = smtk::model::Model(mgr,uid);
      }
    else
      {
      translated = smtk::model::SESSION_ENTITY_RECORD;
      model = mgr->insertModel(
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

      translated |= smtk::model::SESSION_ENTITY_ARRANGED;
      }

    this->addProperties(model, body);
    translated |= smtk::model::SESSION_PROPERTIES;

    this->declareDanglingEntity(model, translated);
    return model;
    }
  return smtk::model::Model();
}

/// Given a CMB \a group tagged with \a uid, create a record in \a mgr for it.
smtk::model::Group Session::addGroupToManager(
  const smtk::common::UUID& uid,
  vtkDiscreteModelEntityGroup* group,
  smtk::model::ManagerPtr mgr,
  int relDepth)
{
  if (group)
    {
    smtk::model::Group result;
    smtk::model::SessionInfoBits translated;
    bool already;
    if ((already = mgr->findEntity(uid, false) ? true : false) || relDepth < 0)
      {
      translated = already ? smtk::model::SESSION_ENTITY_ARRANGED : smtk::model::SESSION_NOTHING;
      result = smtk::model::Group(mgr, uid);
      }
    else
      {
      translated = smtk::model::SESSION_ENTITY_ARRANGED;
      result = mgr->insertGroup(uid);
      }
    if (relDepth >= 0)
      {
      this->addEntities(result, group->NewModelEntityIterator(), AddEntityToGroupHelper(), relDepth - 1);
      }

    this->addProperties(result, group);
    translated |= smtk::model::SESSION_PROPERTIES;

    this->declareDanglingEntity(result, translated);
    return result;
    }
  return smtk::model::Group();
}

/// Given a CMB \a material tagged with \a uid, create a record in \a mgr for it.
smtk::model::Group Session::addMaterialToManager(
  const smtk::common::UUID& uid,
  vtkModelMaterial* material,
  smtk::model::ManagerPtr mgr,
  int relDepth)
{
  if (material)
    {
    smtk::model::SessionInfoBits translated;
    smtk::model::Group result;
    bool already;
    if ((already = mgr->findEntity(uid, false) ? true : false) || relDepth < 0)
      {
      translated = already ? smtk::model::SESSION_EVERYTHING : smtk::model::SESSION_NOTHING;
      result = smtk::model::Group(mgr, uid);
      }
    else
      {
      translated = smtk::model::SESSION_ENTITY_ARRANGED;
      result = mgr->insertGroup(uid);
      }
    if (relDepth >= 0)
      {
      // Add material relations and arrangements
      translated |= smtk::model::SESSION_ENTITY_ARRANGED;
      }

    // Add material properties:
    this->addProperties(result, material);
    translated |= smtk::model::SESSION_PROPERTIES;

    this->declareDanglingEntity(result, translated);
    return result;
    }
  return smtk::model::Group();
}

/// Given a CMB \a coFace tagged with \a uid, create a record in \a mgr for it.
smtk::model::FaceUse Session::addFaceUseToManager(
  const smtk::common::UUID& uid,
  vtkModelFaceUse* coFace,
  smtk::model::ManagerPtr mgr,
  int relDepth)
{
  if (coFace)
    {
    smtk::model::FaceUse result;
    smtk::model::SessionInfoBits translated;
    bool already;
    smtk::model::Face matchingFace(
      mgr, this->findOrSetEntityUUID(coFace->GetModelFace()));
    if ((already = mgr->findEntity(uid, false) ? true : false))
      {
      translated = already ? smtk::model::SESSION_ENTITY_ARRANGED : smtk::model::SESSION_NOTHING;
      result = smtk::model::FaceUse(mgr, uid);
      }
    else
      {
      translated = smtk::model::SESSION_ENTITY_ARRANGED;
      // vtkModelFaceUse does not provide any orientation/sense info,
      // so we check the face to find its orientation. Blech.
      result = mgr->setFaceUse(
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

        this->addCMBEntityToManager(matchingFace.entity(), coFace->GetModelFace(), mgr, relDepth - 1);
        this->addCMBEntityToManager(shellId, shellUse, mgr, relDepth - 1);
        vtkModelItemIterator* loopIt = coFace->NewLoopUseIterator();
        smtk::model::Loops loops;
        for (loopIt->Begin(); !loopIt->IsAtEnd(); loopIt->Next())
          {
          vtkModelItem* loop = loopIt->GetCurrentItem();
          smtk::common::UUID loopId = this->findOrSetEntityUUID(loop);
          this->addCMBEntityToManager(loopId, loop, mgr, relDepth - 1);
          loops.push_back(smtk::model::Loop(mgr, loopId));
          }
        loopIt->Delete();
        smtk::model::FaceUse faceUse(mgr, uid);
        faceUse.addShellEntities(loops);
        faceUse.setBoundingShellEntity(smtk::model::ShellEntity(mgr,shellId));
        }
      }

    this->addProperties(result, coFace);
    translated |= smtk::model::SESSION_PROPERTIES;

    this->declareDanglingEntity(result, translated);
    return result;
    }
  return smtk::model::FaceUse();
}

/// Given a CMB \a coEdge tagged with \a uid, create a record in \a mgr for it.
smtk::model::EdgeUse Session::addEdgeUseToManager(
  const smtk::common::UUID& uid,
  vtkModelEdgeUse* coEdge,
  smtk::model::ManagerPtr mgr,
  int relDepth)
{
  if (!coEdge)
    return smtk::model::EdgeUse();

  smtk::model::SessionInfoBits translated = smtk::model::SESSION_NOTHING;
  bool already;
  smtk::model::EdgeUse result(mgr, uid);
  if ((already = mgr->findEntity(uid, false) ? true : false))
    {
    translated = already ? smtk::model::SESSION_ENTITY_TYPE : smtk::model::SESSION_NOTHING;
    }

  if (relDepth >= 0)
    {
    smtk::model::Edge matchingEdge(
      mgr, this->findOrSetEntityUUID(coEdge->GetModelEdge()));
    if (mgr->findEntity(matchingEdge.entity(), false) == NULL)
      { // Force the addition of the parent edge to the model.
      this->addEdgeToManager(matchingEdge.entity(), coEdge->GetModelEdge(), mgr, 0);
      }
    // Now create the edge use with the proper relation:
    mgr->setEdgeUse(
      uid, matchingEdge, senseOfEdgeUse(coEdge),
      coEdge->GetDirection() ? smtk::model::POSITIVE : smtk::model::NEGATIVE);
    // Finally, create its loop.
    vtkModelLoopUse* loopUse = coEdge->GetModelLoopUse();
    if (loopUse)
      {
      smtk::common::UUID luid = this->findOrSetEntityUUID(loopUse);
      smtk::model::Loop lpu = this->addLoopToManager(luid, loopUse, mgr, relDepth - 1);
      }

    translated |= smtk::model::SESSION_ENTITY_RELATIONS;
    translated |= smtk::model::SESSION_ENTITY_ARRANGED;

    this->addProperties(result, coEdge);
    translated |= smtk::model::SESSION_PROPERTIES;
    }
  return result;
}

/// Given a CMB \a coVertex tagged with \a uid, create a record in \a mgr for it.
smtk::model::VertexUse Session::addVertexUseToManager(
  const smtk::common::UUID& uid,
  vtkModelVertexUse* coVertex,
  smtk::model::ManagerPtr mgr,
  int relDepth)
{
  if (coVertex && !mgr->findEntity(uid, false))
    {
    smtk::model::VertexUse result(mgr, uid);
    smtk::model::SessionInfoBits translated = smtk::model::SESSION_NOTHING;
    if (relDepth >= 0)
      {
      // Add coVertex relations and arrangements
      }

    this->addProperties(result, coVertex);
    translated |= smtk::model::SESSION_PROPERTIES;

    return result;
    }
  return smtk::model::VertexUse();
}

/// Given a CMB \a shell tagged with \a uid, create a record in \a mgr for it.
smtk::model::Shell Session::addShellToManager(
  const smtk::common::UUID& uid,
  vtkModelShellUse* shell,
  smtk::model::ManagerPtr mgr,
  int relDepth)
{
  if (shell && !mgr->findEntity(uid, false))
    {
    smtk::model::Shell result(mgr, uid);
    smtk::model::SessionInfoBits translated = smtk::model::SESSION_NOTHING;
    if (relDepth >= 0)
      {
      // Add shell relations and arrangements
      }

    this->addProperties(result, shell);
    translated |= smtk::model::SESSION_PROPERTIES;

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

/// Given a CMB \a loop tagged with \a uid, create a record in \a mgr for it.
smtk::model::Loop Session::addLoopToManager(
  const smtk::common::UUID& uid,
  vtkModelLoopUse* refLoop,
  smtk::model::ManagerPtr mgr,
  int relDepth)
{
  vtkModelFace* refFace;
  if (refLoop && (refFace = refLoop->GetModelFace()) && !mgr->findEntity(uid, false))
    {
    smtk::model::SessionInfoBits translated = smtk::model::SESSION_NOTHING;
    // Insert the loop, which means inserting the face use and face regardless of relDepth,
    // because the loop's orientation and nesting must be arranged relative to them.
    smtk::common::UUID fid = this->findOrSetEntityUUID(refFace);
    this->addFaceToManager(fid, refFace, mgr, relDepth - 1);
    // Find the face *use* this loop belongs to and transcribe it.
    // Note that loop may be the child of a face use OR another loop (which we must then transcribe).
    vtkModelLoopUse* refLoopParent = NULL;
    int faceUseOrientation = -1;
    vtkModelFaceUse* refFaceUse = locateLoopInFace(refLoop, faceUseOrientation, refLoopParent);
    if (!refFaceUse || faceUseOrientation < 0) return smtk::model::Loop();
    //smtk::common::UUID fuid = this->findOrSetEntityUUID(refFaceUse);
    smtk::model::FaceUse faceUse = this->addFaceUseToManager(fid, refFaceUse, mgr, 0);
    smtk::model::Loop loop;
    if (refLoopParent)
      {
      smtk::common::UUID pluid = this->findOrSetEntityUUID(refLoopParent);
      smtk::model::Loop parentLoop = this->addLoopToManager(pluid, refLoopParent, mgr, 0);
      loop = mgr->setLoop(uid, parentLoop);
      }
    else
      {
      loop = mgr->setLoop(uid, faceUse);
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
        this->addEdgeUseToManager(euid, eu, mgr, relDepth - 1);
        }
      }

    this->addProperties(loop, refLoop);
    translated |= smtk::model::SESSION_PROPERTIES;

    return loop;
    }
  return smtk::model::Loop();
}

/// Given a CMB \a refVolume tagged with \a uid, create a record in \a mgr for it.
smtk::model::Volume Session::addVolumeToManager(
  const smtk::common::UUID& uid,
  vtkModelRegion* refVolume,
  smtk::model::ManagerPtr mgr,
  int relDepth)
{
  if (refVolume && !mgr->findEntity(uid, false))
    {
    smtk::model::Volume result(mgr->insertVolume(uid));
    smtk::model::SessionInfoBits translated = smtk::model::SESSION_NOTHING;
    if (relDepth >= 0)
      {
      // Add refVolume relations and arrangements
      this->addEntities(result, refVolume->NewAdjacentModelFaceIterator(), AddRawRelationHelper(), relDepth - 1);
      this->addEntities(result, refVolume->NewIterator(vtkModelShellUseType), AddVolumeUseToVolumeHelper(), relDepth - 1);
      }

    this->addProperties(result, refVolume);
    translated |= smtk::model::SESSION_PROPERTIES;

    return result;
    }
  return smtk::model::Volume();
}

/// Given a CMB \a refFace tagged with \a uid, create a record in \a mgr for it.
smtk::model::Face Session::addFaceToManager(
  const smtk::common::UUID& uid,
  vtkModelFace* refFace,
  smtk::model::ManagerPtr mgr,
  int relDepth)
{
  if (!refFace)
    return smtk::model::Face();

  smtk::model::SessionInfoBits actual = smtk::model::SESSION_NOTHING;
  smtk::model::Face mutableEntityRef(mgr, uid);
  if (!mutableEntityRef.isValid())
    mutableEntityRef.manager()->insertFace(uid);
  actual |= smtk::model::SESSION_ENTITY_TYPE;

  if (relDepth >= 0)
    { // Add refFace relations and arrangements
    // If face uses exist, add them to the session.
    vtkModelFaceUse* fu;
    for (int i = 0; i < 2; ++i)
      {
      fu = refFace->GetModelFaceUse(i); // 0 = negative, 1 = positive
      if (fu)
        {
        smtk::common::UUID fuid = this->findOrSetEntityUUID(fu);
        this->addFaceUseToManager(fuid, fu, mgr, relDepth - 1);
        // Now, since we are the "higher" end of the relationship,
        // arrange the use wrt ourself:
        mgr->findCreateOrReplaceCellUseOfSenseAndOrientation(
          uid, 0, i ? smtk::model::POSITIVE : smtk::model::NEGATIVE, fuid);
        }
      }

    // Add a reference to the volume(s) directly (with no relationship)
    int nvols = refFace->GetNumberOfModelRegions();
    for (int i = 0; i < nvols; ++i)
      {
      vtkModelRegion* vol = refFace->GetModelRegion(i);
      if (vol)
        {
        Volume v(mgr, this->findOrSetEntityUUID(vol));
        mutableEntityRef.addRawRelation(v);
        }
      }

    std::vector<vtkModelEdge*> edges;
    refFace->GetModelEdges(edges);
    this->addEntityArray(mutableEntityRef, edges, AddRawRelationHelper(), relDepth - 1);
    actual |= smtk::model::SESSION_ENTITY_RELATIONS;
    actual |= smtk::model::SESSION_ARRANGEMENTS;

    // Add geometry, if any.
    this->addTessellation(mutableEntityRef, refFace);
    actual |= smtk::model::SESSION_TESSELLATION;
    }

  this->addProperties(mutableEntityRef, refFace);
  actual |= smtk::model::SESSION_PROPERTIES;

  return mutableEntityRef;
}

/// Given a CMB \a refEdge tagged with \a uid, create a record in \a mgr for it.
smtk::model::Edge Session::addEdgeToManager(
  const smtk::common::UUID& uid,
  vtkModelEdge* refEdge,
  smtk::model::ManagerPtr mgr,
  int relDepth)
{
  if (!refEdge)
    return smtk::model::Edge();

  smtk::model::SessionInfoBits actual = smtk::model::SESSION_NOTHING;
  smtk::model::Edge mutableEntityRef(mgr, uid);
  if (!mutableEntityRef.isValid())
    mutableEntityRef.manager()->insertEdge(uid);
  actual |= smtk::model::SESSION_ENTITY_TYPE;

  if (relDepth >= 0)
    { // Add refEdge relations and arrangements
    // If edge uses exist, add them to the session.
    vtkModelEdgeUse* eu;
    for (int i = 0; i < refEdge->GetNumberOfModelEdgeUses(); ++i)
      {
      eu = refEdge->GetModelEdgeUse(i); // 0 = negative, 1 = positive
      if (eu)
        {
        smtk::common::UUID euid = this->findOrSetEntityUUID(eu);
        this->addEdgeUseToManager(euid, eu, mgr, relDepth - 1);
        // Now, since we are the "higher" end of the relationship,
        // arrange the use wrt ourself:
        mgr->findCreateOrReplaceCellUseOfSenseAndOrientation(
          uid, 0, i ? smtk::model::POSITIVE : smtk::model::NEGATIVE, euid);
        }
      }

    // Add a reference to the face(s) directly (with no relationship)
    vtkModelItemIterator* faceIt = refEdge->NewAdjacentModelFaceIterator();
    for (faceIt->Begin(); !faceIt->IsAtEnd(); faceIt->Next())
      {
      vtkModelFace* refFace = vtkModelFace::SafeDownCast(faceIt->GetCurrentItem());
      if (!refFace)
        continue;

      smtk::model::Face f(mgr, this->findOrSetEntityUUID(refFace));
      mutableEntityRef.addRawRelation(f);
      }

    // Add a reference to the vertices directly (with no relationship)
    int numVerts = refEdge->GetNumberOfModelVertexUses();
    for (int i = 0; i < numVerts; ++i)
      {
      vtkModelVertex* refVert = refEdge->GetAdjacentModelVertex(i);
      if (!refVert)
        continue;

      smtk::model::Vertex v(mgr, this->findOrSetEntityUUID(refVert));
      this->addVertexToManager(v.entity(), refVert, mgr, relDepth - 1);
      mutableEntityRef.addRawRelation(v);
      }

    actual |= smtk::model::SESSION_ENTITY_RELATIONS;
    actual |= smtk::model::SESSION_ARRANGEMENTS;

    // Add geometry, if any.
    this->addTessellation(mutableEntityRef, refEdge);
    actual |= smtk::model::SESSION_TESSELLATION;
    }

  this->addProperties(mutableEntityRef, refEdge);
  actual |= smtk::model::SESSION_PROPERTIES;

  return mutableEntityRef;
}

/// Given a CMB \a refVertex tagged with \a uid, create a record in \a mgr for it.
smtk::model::Vertex Session::addVertexToManager(
  const smtk::common::UUID& uid,
  vtkModelVertex* refVertex,
  smtk::model::ManagerPtr mgr,
  int relDepth)
{
  if (refVertex && !mgr->findEntity(uid, false))
    {
    smtk::model::Vertex result(mgr->insertVertex(uid));
    smtk::model::SessionInfoBits translated = smtk::model::SESSION_NOTHING;
    if (relDepth >= 0)
      {
      // Add refVertex relations and arrangements
      vtkModelItemIterator* vuit = refVertex->NewModelVertexUseIterator();
      for (vuit->Begin(); !vuit->IsAtEnd(); vuit->Next())
        {
        vtkModelVertexUse* vu =
          vtkModelVertexUse::SafeDownCast(
            vuit->GetCurrentItem());
        if (!vu)
          continue;

        smtk::common::UUID vuid = this->findOrSetEntityUUID(vu);
        this->addVertexUseToManager(vuid, vu, mgr, relDepth - 1);
        }
      vuit->Delete();

      // Add reference to edge(s) directly (with no relationship)
      vtkModelItemIterator* eit = refVertex->NewAdjacentModelEdgeIterator();
      for (eit->Begin(); !eit->IsAtEnd(); eit->Next())
        {
        vtkModelItem* ee = eit->GetCurrentItem();
        smtk::common::UUID eid = this->findOrSetEntityUUID(ee);
        result.addRawRelation(smtk::model::Edge(mgr, eid));
        }
      eit->Delete();

      // Add geometry, if any.
      this->addTessellation(result, refVertex);
      }

    this->addProperties(result, refVertex);
    translated |= smtk::model::SESSION_PROPERTIES;

    return result;
    }
  return smtk::model::Vertex();
}

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#include "smtk/bridge/discrete/Session_json.h"
smtkImplementsModelingKernel(
  discrete,
  Session_json,
  SessionHasNoStaticSetup,
  smtk::bridge::discrete::Session,
  true /* inherit "universal" operators */
);

// Force these operators to be registered whenever the session is used:
smtkComponentInitMacro(smtk_discrete_read_operator);
smtkComponentInitMacro(smtk_discrete_merge_operator);
smtkComponentInitMacro(smtk_discrete_split_face_operator);
// smtkComponentInitMacro(smtk_discrete_create_edges_operator);
smtkComponentInitMacro(smtk_discrete_import_operator);
smtkComponentInitMacro(smtk_discrete_entity_group_operator);
smtkComponentInitMacro(smtk_discrete_grow_operator);
smtkComponentInitMacro(smtk_discrete_write_operator);
