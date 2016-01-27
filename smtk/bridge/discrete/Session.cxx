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
#include "smtk/bridge/discrete/ArrangementHelper.h"
#include "smtk/bridge/discrete/BathymetryHelper.h"

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
#include "smtk/model/Instance.h"

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
  this->m_bathymetryHelper = new smtk::bridge::discrete::BathymetryHelper();
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
  for (mbit = this->m_modelsToSessions.begin(); mbit != this->m_modelsToSessions.end(); )
    {
    if (mbit->second.lock().get() == this)
      {
      smtk::common::UUID modelId = this->findOrSetEntityUUID(mbit->first);
      this->m_modelsToSessions.erase(mbit++);
      vtkSmartPointer<vtkDiscreteModelWrapper> modelPtr = this->m_modelIdsToRefs[modelId];
      this->m_modelIdsToRefs.erase(modelId);
      this->m_modelRefsToIds.erase(modelPtr);
      }
    else
      {
      ++mbit;
      }
    }
  this->m_itemWatcher->session = NULL;
  this->m_itemWatcher->Delete();
  if(this->m_bathymetryHelper)
    {
    this->m_bathymetryHelper->clear();
    delete this->m_bathymetryHelper;
    }
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
    this->m_modelsToSessions.find(model);
  if (sessionIt == this->m_modelsToSessions.end())
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
    wri->Write(*mit, this);
    std::cout << "Wrote " << fname.str() << "\n";
    }

  return 0;
}

vtkDiscreteModelWrapper* Session::findModelEntity(const smtk::common::UUID& uid) const
{
  std::map<smtk::common::UUID,vtkSmartPointer<vtkDiscreteModelWrapper> >::const_iterator it;
  if ((it = this->m_modelIdsToRefs.find(uid)) != this->m_modelIdsToRefs.end())
    return it->second.GetPointer();
  return NULL;
}

/**\brief Find the kernel entity corresponding to \a entRef and add a record for it to \a entRef's manager.
  *
  * No bidirectional relationships or arrangements should be added
  * at this point since there is no guarantee that their records
  * exist in the manager.
  */
Entity* Session::addEntityRecord(const smtk::model::EntityRef& entRef)
{
  vtkModelItem* ent = this->entityForUUID(entRef.entity());
  if (!ent)
    {
    smtkErrorMacro(this->log(), "Entity " << entRef.entity() << " has no matching CMB model item");
    return NULL;
    }

  vtkModel* modelEntity = dynamic_cast<vtkModel*>(ent);
  vtkModelGeometricEntity* cellEntity = dynamic_cast<vtkModelGeometricEntity*>(ent);
  vtkModelEntity* otherEntity = dynamic_cast<vtkModelEntity*>(ent);
  if (modelEntity)
    {
    double bds[6];
    modelEntity->GetBounds(bds);
    int embeddingDim = 0;
    for (int i = 0; i < 3; ++i)
      if (bds[2*i] < bds[2*i + 1])
        ++embeddingDim;
    if (embeddingDim < 1)
      embeddingDim = 3; // Default to 3D model if currently empty.
    entRef.manager()->insertModel(entRef.entity(), modelEntity->GetModelDimension(), embeddingDim); // , name
    }
  else if (cellEntity)
    {
    vtkModelRegion* region = dynamic_cast<vtkModelRegion*>(otherEntity);
    vtkModelFace* face = dynamic_cast<vtkModelFace*>(otherEntity);
    vtkModelEdge* edge = dynamic_cast<vtkModelEdge*>(otherEntity);
    vtkModelVertex* vert = dynamic_cast<vtkModelVertex*>(otherEntity);
    if (region)
      entRef.manager()->addEntityOfTypeAndDimensionWithUUID(entRef.entity(), CELL_3D, 3);
    else if (face)
      {
      entRef.manager()->addEntityOfTypeAndDimensionWithUUID(entRef.entity(), CELL_2D, 2);
      // Now get rid of mandatory positive/negative uses since we overwrite those
      entRef.manager()->clearArrangements(entRef.entity());
      }
    else if (edge)
      entRef.manager()->addEntityOfTypeAndDimensionWithUUID(entRef.entity(), CELL_1D, 1);
    else if (vert)
      entRef.manager()->addEntityOfTypeAndDimensionWithUUID(entRef.entity(), CELL_0D, 0);
    else
      {
      smtkErrorMacro(this->log(),
        "Unknown vtkModelGeometricEntity subclass \""
        << cellEntity->GetClassName() << "\" encountered. Ignoring.");
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
    bool isUse = false;
    if (faceUse) { entRef.manager()->addEntityOfTypeAndDimensionWithUUID(entRef.entity(), FACE_USE, 2); isUse = true; }
    else if (edgeUse) { entRef.manager()->addEntityOfTypeAndDimensionWithUUID(entRef.entity(), EDGE_USE, 1); isUse = true; }
    else if (vertUse) { entRef.manager()->addEntityOfTypeAndDimensionWithUUID(entRef.entity(), VERTEX_USE, 0); isUse = true; }
    else if (group)
      {
      int groupFlags;
      switch (group->GetEntityType())
        {
      case vtkModelType: groupFlags = smtk::model::MODEL_ENTITY; break;
      case vtkModelVertexType: groupFlags = smtk::model::VERTEX; break;
      case vtkModelVertexUseType: groupFlags = smtk::model::VERTEX_USE; break;
      case vtkModelEdgeType: groupFlags = smtk::model::EDGE; break;
      case vtkModelEdgeUseType: groupFlags = smtk::model::EDGE_USE; break;
      case vtkModelLoopUseType: groupFlags = smtk::model::LOOP; break;
      case vtkModelFaceType: groupFlags = smtk::model::FACE; break;
      case vtkModelFaceUseType: groupFlags = smtk::model::FACE_USE; break;
      case vtkModelShellUseType: groupFlags = smtk::model::SHELL; break;
      case vtkModelRegionType: groupFlags = smtk::model::VOLUME; break;
      default:
      case -1:
        groupFlags = 0;
        break;
        }
      entRef.manager()->insertGroup(entRef.entity(), groupFlags);
      }
    else if (material) entRef.manager()->insertGroup(entRef.entity(), smtk::model::MODEL_DOMAIN);
    else if (shell) entRef.manager()->addEntityOfTypeAndDimensionWithUUID(entRef.entity(), SHELL, -1);
    else if (loop) entRef.manager()->addEntityOfTypeAndDimensionWithUUID(entRef.entity(), LOOP, -1);
    else
      {
      smtkErrorMacro(this->log(),
        "Unknown vtkModel subclass \""
        << otherEntity->GetClassName() << "\" encountered. Ignoring.");
      }
    (void)isUse;
    }
  return entRef.manager()->findEntity(entRef.entity());
}

/// Create a helper specific to the SMTK discrete kernel.
smtk::model::ArrangementHelper* Session::createArrangementHelper()
{
  return new ArrangementHelper;
}

/**\brief Add cells related to the given \a cell.
  *
  * The cells may be related as bounding \a cell, being bounded by \a cell,
  * embedded in \a cell, or embedded by \a cell.
  */
int Session::findOrAddCellAdjacencies(
  const smtk::model::CellEntity& cell,
  SessionInfoBits request,
  smtk::model::ArrangementHelper* hlp)
{
  (void)request;
  ArrangementHelper* helper = dynamic_cast<ArrangementHelper*>(hlp);
  int numEnts = 0;
  vtkModelItem* modelCell = this->entityForUUID(cell.entity());

  vtkModelRegion* modelRegion = dynamic_cast<vtkModelRegion*>(modelCell);
  vtkModelFace* modelFace = dynamic_cast<vtkModelFace*>(modelCell);
  vtkModelEdge* modelEdge = dynamic_cast<vtkModelEdge*>(modelCell);
  vtkModelVertex* modelVertex = dynamic_cast<vtkModelVertex*>(modelCell);
  if (modelRegion)
    {
    numEnts += this->addEntities(cell, modelRegion->NewAdjacentModelFaceIterator(), smtk::model::SUPERSET_OF, helper);
    }
  if (modelFace)
    {
    for (int r = 0; r < modelFace->GetNumberOfModelRegions(); ++r, ++numEnts)
      this->addEntity(modelFace->GetModelRegion(r), cell, smtk::model::SUPERSET_OF, helper);
    numEnts += this->addEntities(cell, modelFace->NewAdjacentModelEdgeIterator(), smtk::model::SUPERSET_OF, helper);
    }
  if (modelEdge)
    {
    numEnts += this->addEntities(modelEdge->NewAdjacentModelFaceIterator(), cell, smtk::model::SUPERSET_OF, helper);
    int nv = modelEdge->GetNumberOfModelVertexUses();
    for (int v = 0; v < nv; ++v, ++numEnts)
      this->addEntity(cell, modelEdge->GetAdjacentModelVertex(v), smtk::model::SUPERSET_OF, helper);
    }
  if (modelVertex)
    {
    numEnts += this->addEntities(modelVertex->NewAdjacentModelEdgeIterator(), cell, smtk::model::SUPERSET_OF, helper);
    }

  // Also consider included cells (i.e., not on the boundary but interior to the cell's point locus)
  if (modelCell->GetNumberOfAssociations(vtkModelRegionType))
    { // Add regions to model
    numEnts = this->addEntities(cell, modelCell->NewIterator(vtkModelRegionType), smtk::model::INCLUDES, helper);
    }
  else if(modelCell->GetNumberOfAssociations(vtkModelFaceType))
    { // Add faces to model
    numEnts = this->addEntities(cell, modelCell->NewIterator(vtkModelFaceType), smtk::model::INCLUDES, helper);
    }
  else if(modelCell->GetNumberOfAssociations(vtkModelEdgeType))
    { // Add edges to model
    numEnts = this->addEntities(cell, modelCell->NewIterator(vtkModelEdgeType), smtk::model::INCLUDES, helper);
    }
  else if(modelCell->GetNumberOfAssociations(vtkModelVertexType))
    { // Add vertices to model
    numEnts = this->addEntities(cell, modelCell->NewIterator(vtkModelVertexType), smtk::model::INCLUDES, helper);
    }

  return numEnts;
}

template<typename T>
struct LookupSenseOfUse
{
  LookupSenseOfUse(ArrangementHelper* helper)
    : m_helper(helper)
    {
    }

  bool operator () (vtkModelItem* dscEntity, int& sense, Orientation& orientation)
    {
    T* ent = dynamic_cast<T*>(dscEntity);
    if (!ent)
      return false;
    orientation = ent->GetDirection() ? smtk::model::POSITIVE : smtk::model::NEGATIVE;
    sense = this->m_helper->findOrAssignSense(ent);
    return true;
    }

  ArrangementHelper* m_helper;
};

template<>
struct LookupSenseOfUse<vtkModelVertexUse>
{
  LookupSenseOfUse(ArrangementHelper*)
    {
    }

  bool operator () (vtkModelItem* dscEntity, int& sense, Orientation& orientation)
    {
    vtkModelVertexUse* ent = dynamic_cast<vtkModelVertexUse*>(dscEntity);
    if (!ent)
      return false;
    orientation = smtk::model::UNDEFINED;
    sense = 0; // FIXME
    return true;
    }
};

struct LookupSenseForShellEnt
{
  LookupSenseForShellEnt(ArrangementHelper* helper)
    : m_helper(helper)
    {
    }

  bool operator () (vtkModelItem* dscEntity, int& sense, Orientation& orientation)
    {
    vtkModelShellUse* ent = dynamic_cast<vtkModelShellUse*>(dscEntity);
    if (!ent)
      return false;
    // FIXME: In the future, this should detect whether a shell is an
    //        inner or outer shell of a volume and use that to decide on
    //        the sense relative to the volume use.
    //vtkModelRegion* parent = ent->GetModelRegion();
    orientation = smtk::model::POSITIVE;
    sense = 0; //this->m_helper->findOrAssignSense(ent);
    return true;
    }

  ArrangementHelper* m_helper;
};

int Session::findOrAddCellUses(
  const smtk::model::CellEntity& cell,
  SessionInfoBits request,
  smtk::model::ArrangementHelper* hlp)
{
  (void)request;
  ArrangementHelper* helper = dynamic_cast<ArrangementHelper*>(hlp);
  int numEnts = 0;
  vtkModelItem* modelCell = this->entityForUUID(cell.entity());
  vtkModelRegion* modelRegion = dynamic_cast<vtkModelRegion*>(modelCell);
  vtkModelFace* modelFace = dynamic_cast<vtkModelFace*>(modelCell);
  vtkModelEdge* modelEdge = dynamic_cast<vtkModelEdge*>(modelCell);
  vtkModelVertex* modelVertex = dynamic_cast<vtkModelVertex*>(modelCell);
  if (modelRegion)
    {
    // NB: The discrete session does not have volume uses... we use the helper to
    //     store an extra UUID.
    smtk::model::EntityRef volumeUse(cell.manager(), helper->useForRegion(modelRegion));
    /*
    if (!helper->isMarked(childRef))
      this->transcribe(childRef, smtk::model::SESSION_EVERYTHING, false, -1);
     */
    this->addEntityRecord(volumeUse);
    this->findOrAddRelatedEntities(volumeUse, smtk::model::SESSION_EVERYTHING, helper);
    helper->addArrangement(
      cell, smtk::model::HAS_USE, volumeUse, /* sense */ 0, smtk::model::POSITIVE);
    }
  else if (modelFace)
    {
    this->addEntity(cell, modelFace->GetModelFaceUse(0), smtk::model::HAS_USE, helper, 0, smtk::model::NEGATIVE);
    this->addEntity(cell, modelFace->GetModelFaceUse(1), smtk::model::HAS_USE, helper, 0, smtk::model::POSITIVE);
    numEnts = 2;
    }
  else if (modelEdge)
    {
    LookupSenseOfUse<vtkModelEdgeUse> senseLookup(helper);
    numEnts = this->addEntities(cell, modelEdge->NewModelEdgeUseIterator(), smtk::model::HAS_USE, helper, senseLookup);
    }
  else if (modelVertex)
    {
    LookupSenseOfUse<vtkModelVertexUse> senseLookup(helper);
    numEnts = this->addEntities(cell, modelVertex->NewModelVertexUseIterator(), smtk::model::HAS_USE, helper, senseLookup);
    }
  return numEnts;
}

int Session::findOrAddOwningCell(
  const smtk::model::UseEntity& entRef,
  SessionInfoBits request,
  smtk::model::ArrangementHelper* hlp)
{
  (void)request;
  ArrangementHelper* helper = dynamic_cast<ArrangementHelper*>(hlp);
  smtk::model::EntityRef cell;
  if (entRef.isVolumeUse())
    {
    cell = smtk::model::Volume(
      entRef.manager(),
      this->findOrSetEntityUUID(
        helper->regionFromUseId(entRef.entity())));
    this->addEntityRecord(cell);
    this->findOrAddRelatedEntities(cell, smtk::model::SESSION_EVERYTHING, helper);
    helper->addArrangement(cell, smtk::model::HAS_CELL, entRef, 0, smtk::model::POSITIVE);
    }
  else
    {
    int sense;
    smtk::model::Orientation orientation;
    vtkModelItem* dscUse = this->entityForUUID(entRef.entity());
    vtkModelVertexUse* dscVertexUse = dynamic_cast<vtkModelVertexUse*>(dscUse);
    vtkModelEdgeUse* dscEdgeUse = dynamic_cast<vtkModelEdgeUse*>(dscUse);
    vtkModelFaceUse* dscFaceUse = dynamic_cast<vtkModelFaceUse*>(dscUse);
    if (dscFaceUse)
      {
      cell = smtk::model::EntityRef(entRef.manager(), this->findOrSetEntityUUID(dscFaceUse->GetModelFace()));
      sense = 0;
      orientation =
        (dscFaceUse->GetModelFace()->GetModelFaceUse(1) == dscFaceUse ?
          smtk::model::POSITIVE :
          smtk::model::NEGATIVE);
      }
    else if (dscEdgeUse)
      {
      cell = smtk::model::EntityRef(entRef.manager(), this->findOrSetEntityUUID(dscEdgeUse->GetModelEdge()));
      sense = helper->findOrAssignSense(dscEdgeUse);
      orientation = dscEdgeUse->GetDirection() ? smtk::model::POSITIVE : smtk::model::NEGATIVE;
      }
    else if (dscVertexUse)
      {
      cell = smtk::model::EntityRef(entRef.manager(), this->findOrSetEntityUUID(dscVertexUse->GetModelVertex()));
      sense = 0; // FIXME (see also LookupSenseOfUse)
      orientation = smtk::model::UNDEFINED;
      }
    else
      {
      sense = -1;
      }
    if (sense < 0)
      return 0;

    this->addEntity(cell, dscUse, smtk::model::HAS_USE, helper, sense, orientation);
    // addEntity searches for relationships of dscUse, but we need to add the cell, not the use:
    //this->addEntityRecord(cell, smtk::model::SESSION_EVERYTHING, helper);
    this->findOrAddRelatedEntities(cell, smtk::model::SESSION_EVERYTHING, helper);
    }
  return 1;
}

int Session::findOrAddShellAdjacencies(
  const smtk::model::UseEntity& entRef,
  SessionInfoBits request,
  smtk::model::ArrangementHelper* hlp)
{
  (void)request;
  int numEnts = 0;
  ArrangementHelper* helper = dynamic_cast<ArrangementHelper*>(hlp);
  smtk::model::EntityRef cell;
  if (entRef.isVolumeUse())
    {
    vtkModelRegion* region = helper->regionFromUseId(entRef.entity());
    LookupSenseForShellEnt senseLookup(helper);
    cell = smtk::model::Volume(
      entRef.manager(),
      this->findOrSetEntityUUID(region));
    numEnts += this->addEntities(entRef, region->NewModelShellUseIterator(), smtk::model::INCLUDES, helper, senseLookup);
    }
  else
    {
    vtkModelItem* dscUse = this->entityForUUID(entRef.entity());
    vtkModelFaceUse* dscFaceUse = dynamic_cast<vtkModelFaceUse*>(dscUse);
    vtkModelEdgeUse* dscEdgeUse = dynamic_cast<vtkModelEdgeUse*>(dscUse);
    vtkModelVertexUse* dscVertexUse = dynamic_cast<vtkModelVertexUse*>(dscUse);
    if (dscFaceUse)
      {
      // Add relationship to parent shell
      vtkModelShellUse* dscShell = dscFaceUse->GetModelShellUse();
      if (dscShell)
        {
        smtk::model::Shell parent(entRef.manager(), this->findOrSetEntityUUID(dscShell));
        this->addEntity(parent, dscFaceUse, smtk::model::HAS_USE, helper);
        ++numEnts;
        }

      // Add relationship to child loop
      vtkModelLoopUse* dscLoop = dscFaceUse->GetOuterLoopUse();
      if (dscLoop)
        {
        this->addEntity(entRef, dscLoop, smtk::model::INCLUDES, helper);
        ++numEnts;
        }
      }
    else if (dscEdgeUse)
      {
      // Add relationship to parent shell
      vtkModelLoopUse* dscLoop = dscEdgeUse->GetModelLoopUse();
      if (dscLoop)
        {
        smtk::model::Loop parent(entRef.manager(), this->findOrSetEntityUUID(dscLoop));
        this->addEntity(parent, dscEdgeUse, smtk::model::HAS_USE, helper);
        ++numEnts;
        }

      // Have the helper create a "virtual" Chain of vertex uses and add relations to those.
      // Would call this->addEntityRecord(childRef); but there is no matching discrete item...
      // ... so manually insert the chain.
      smtk::model::Chain childRef =
        entRef.manager()->insertChain(helper->chainForEdgeUse(dscEdgeUse));
      this->findOrAddRelatedEntities(childRef, smtk::model::SESSION_EVERYTHING, helper);
      helper->addArrangement(entRef, smtk::model::INCLUDES, childRef);
      numEnts += 2; // loop + outer chain
      }
    else if (dscVertexUse)
      {
      // Add the relationship to the parent edge uses.
      // Unlike other use records, a single vertex use can have multiple parent chains.
      // Because the discrete kernel does not have the concept of a chain (an oriented sequence of vertices
      // that bound an edge use), we look up the edge uses and obtain chains from them.
      vtkModelItemIterator* euit = dscVertexUse->NewModelEdgeUseIterator();
      for (euit->Begin(); !euit->IsAtEnd(); euit->Next(), ++numEnts)
        {
        smtk::model::Chain chain =
          entRef.manager()->insertChain(
            helper->chainForEdgeUse(
              dynamic_cast<vtkModelEdgeUse*>(euit->GetCurrentItem())));
        this->addEntity(chain, dscVertexUse, smtk::model::HAS_USE, helper);
        }
      euit->Delete();

      // VertexUses have no child loops.
      }
    }
  return numEnts;
}

static int FindParentFaceUse(
  vtkModelLoopUse* dscLoop,
  vtkModelFace* face,
  vtkModelFaceUse*& faceUse,
  int& useIndex,
  int& loopIndex)
{
  if (!dscLoop || !face)
    return 0;
  for (useIndex = 0; useIndex < 2; ++useIndex)
    {
    faceUse = face->GetModelFaceUse(useIndex);
    if (!faceUse)
      continue;
    vtkModelItemIterator* loopIt = faceUse->NewLoopUseIterator();
    loopIndex = 0;
    for (loopIt->Begin(); !loopIt->IsAtEnd(); loopIt->Next(), ++loopIndex)
      {
      vtkModelLoopUse* loop = dynamic_cast<vtkModelLoopUse*>(loopIt->GetCurrentItem());
      if (dscLoop == loop)
        {
        loopIt->Delete();
        return 1;
        }
      }
    loopIt->Delete();
    }
  return 0;
}

int Session::findOrAddUseAdjacencies(
  const smtk::model::ShellEntity& entRef,
  SessionInfoBits request,
  smtk::model::ArrangementHelper* hlp)
{
  (void)request;
  int numEnts = 0;
  ArrangementHelper* helper = dynamic_cast<ArrangementHelper*>(hlp);
  smtk::model::EntityRef parentUseOrShell;
  vtkModelItem* dscEnt = this->entityForUUID(entRef.entity());
  vtkModelShellUse* dscShell;
  vtkModelLoopUse* dscLoop;
  vtkModelEdgeUse* dscEdgeUse;
  if (entRef.isShell() && (dscShell = dynamic_cast<vtkModelShellUse*>(dscEnt)))
    {
    vtkModelRegion* region = dscShell->GetModelRegion();
    if (region)
      {
      // Add parent and children of shell.
      // Note that the first shell is the only shell considered an outer shell.
      // All others are considered internal (i.e., a single connected component per volume).
      // Internal shells may not contain other shells (i.e., no islands within voids).
      vtkModelItemIterator* shellIt = region->NewModelShellUseIterator();
      shellIt->Begin();
      if (dscShell == shellIt->GetCurrentItem())
        { // Only add other shells if we are the outer shell
        // Add parent volume-use of shell
        smtk::model::VolumeUse parent(
          entRef.manager(), helper->useForRegion(region));
        this->addEntity(parent, dscShell, smtk::model::INCLUDES, helper);
        ++numEnts;

        for (/*nothing*/; !shellIt->IsAtEnd(); shellIt->Next(), ++numEnts)
          {
          vtkModelShellUse* subshell =
            dynamic_cast<vtkModelShellUse*>(
              shellIt->GetCurrentItem());
          this->addEntity(entRef, subshell, smtk::model::INCLUDES, helper);
          }
        }
      else
        {
        // This shell is the child of another shell; it has no child shells of its own.
        smtk::model::Shell parent(
          entRef.manager(), this->findOrSetEntityUUID(shellIt->GetCurrentItem()));
        this->addEntity(parent, dscShell, smtk::model::INCLUDES, helper);
        ++numEnts;
        }
      }
    // Add child face-uses of shell
    numEnts += this->addEntities(entRef, dscShell->NewModelFaceUseIterator(), smtk::model::HAS_USE, helper);
    }
  else if (entRef.isLoop() && (dscLoop = dynamic_cast<vtkModelLoopUse*>(dscEnt)))
    {
    vtkModelFace* face = dscLoop->GetModelFace();
    if (face)
      {
      // Ugly. Loop only provides parent face, not face use. So we must search all loops of all uses of the face to find our parent.
      vtkModelFaceUse* faceUse;
      int useIndex; // 0 for negative face use, 1 for positive face use
      int loopIndex;
      if (FindParentFaceUse(dscLoop, face, faceUse, useIndex, loopIndex))
        {
        // Add parent and children of loop.
        // Note that the first loop is the only loop considered an outer loop.
        // All others are considered internal (i.e., a single connected component per volume).
        // Internal loops may not contain other loops (i.e., no islands within voids).
        if (dscLoop == faceUse->GetOuterLoopUse())
          {
          // Only add other loops if we are the outer loop
          // Add parent volume-use of loop
          smtk::model::FaceUse parent(
            entRef.manager(), this->findOrSetEntityUUID(faceUse));
          this->addEntity(parent, dscLoop, smtk::model::INCLUDES, helper);
          ++numEnts;

          vtkModelItemIterator* loopIt = faceUse->NewLoopUseIterator();
          for (loopIt->Begin(); !loopIt->IsAtEnd(); loopIt->Next())
            {
            vtkModelLoopUse* subloop =
              dynamic_cast<vtkModelLoopUse*>(
                loopIt->GetCurrentItem());
            if (subloop == dscLoop)
              continue;

            this->addEntity(entRef, subloop, smtk::model::INCLUDES, helper);
            ++numEnts;
            }
          loopIt->Delete();
          }
        else
          { // we are an inner loop use.
          // This loop is the child of another loop; it has no child loops of its own.
          smtk::model::Loop parent(
            entRef.manager(), this->findOrSetEntityUUID(faceUse->GetOuterLoopUse()));
          this->addEntity(parent, dscLoop, smtk::model::INCLUDES, helper);
          ++numEnts;
          }
        }
      }
    // Add child edge-uses of loop
    numEnts += this->addEntities(entRef, dscLoop->NewModelEdgeUseIterator(), smtk::model::HAS_USE, helper);
    }
  else if (entRef.isChain() && (dscEdgeUse = helper->edgeUseFromChainId(entRef.entity())))
    {
    // Add parent edge use to chain
    this->addEntity(dscEdgeUse, entRef, smtk::model::INCLUDES, helper);

    // Add child vertex uses
    numEnts += this->addEntities(entRef, dscEdgeUse->NewIterator(vtkModelVertexUseType), smtk::model::HAS_USE, helper);
    }
  return numEnts;
}

int Session::findOrAddGroupOwner(
  const smtk::model::Group& entRef,
  SessionInfoBits request,
  smtk::model::ArrangementHelper* helper)
{
  (void)request;
  (void)entRef;
  (void)helper;
  return 0;
}

int Session::findOrAddFreeCells(
  const smtk::model::Model& entRef,
  SessionInfoBits request,
  smtk::model::ArrangementHelper* hlp)
{
  (void)request;
  ArrangementHelper* helper = dynamic_cast<ArrangementHelper*>(hlp);
  int numEnts = 0;
  vtkModelItem* body = this->entityForUUID(entRef.entity());
  if (body->GetNumberOfAssociations(vtkModelRegionType))
    { // Add regions to model
    numEnts += this->addEntities(entRef, body->NewIterator(vtkModelRegionType), smtk::model::INCLUDES, helper);
    }
  else if (body->GetNumberOfAssociations(vtkModelFaceType))
    { // Add faces to model
    numEnts += this->addEntities(entRef, body->NewIterator(vtkModelFaceType), smtk::model::INCLUDES, helper);
    }
  else if (body->GetNumberOfAssociations(vtkModelEdgeType))
    { // Add edges to model
    numEnts += this->addEntities(entRef, body->NewIterator(vtkModelEdgeType), smtk::model::INCLUDES, helper);
    }
  else if (body->GetNumberOfAssociations(vtkModelVertexType))
    { // Add vertices to model
    numEnts += this->addEntities(entRef, body->NewIterator(vtkModelVertexType), smtk::model::INCLUDES, helper);
    }

  // In case the model contains floating faces, edges, or vertexes,
  // floating face   : no region associated // only for 3D
  // floating edge   : no face associated   // only for 3D
  // floating vertex : no edge associated   // not defined/handled in discrete model kernel
  if (body->GetNumberOfAssociations(vtkModelRegionType) && // This is only for 3D
          body->GetNumberOfAssociations(vtkModelFaceType))
    {// Add floating faces to model
    vtkModelItemIterator* iter = body->NewIterator(vtkModelFaceType);
    for(iter->Begin();!iter->IsAtEnd();iter->Next())
      {
      vtkModelFace* face =
        vtkModelFace::SafeDownCast(iter->GetCurrentItem());
      if(face && face->GetNumberOfModelRegions() == 0)
        {
        this->addEntity(entRef, face, smtk::model::INCLUDES, helper,
             -1, smtk::model::UNDEFINED);
        ++numEnts;
        }
      }
    iter->Delete();
    }
  if (body->GetNumberOfAssociations(vtkModelRegionType) && // This is only for 3D
          body->GetNumberOfAssociations(vtkModelEdgeType))
    { // Add floating edges to model
    vtkModelItemIterator* iter = body->NewIterator(vtkModelEdgeType);
    for(iter->Begin();!iter->IsAtEnd();iter->Next())
      {
      vtkDiscreteModelEdge* edge =
        vtkDiscreteModelEdge::SafeDownCast(iter->GetCurrentItem());
      if(edge && edge->GetNumberOfAdjacentModelFaces() == 0)
        {
        if(vtkModelRegion* region = edge->GetModelRegion())
          {
          smtk::common::UUID rid = this->findOrSetEntityUUID(region);
          // make the associated region to be the parent of floating edge
          this->addEntity(smtk::model::EntityRef(entRef.manager(), rid),
               edge, smtk::model::INCLUDES, helper,
               -1, smtk::model::UNDEFINED);
          ++numEnts;
          }
        else
          {
          this->addEntity(entRef, edge, smtk::model::INCLUDES, helper,
               -1, smtk::model::UNDEFINED);
          ++numEnts;
          }
        }
      }
    iter->Delete();
    }

  return numEnts;
}

int Session::findOrAddRelatedModels(
  const smtk::model::Model& entRef,
  SessionInfoBits request,
  smtk::model::ArrangementHelper* helper)
{
  (void)request;
  (void)entRef;
  (void)helper;
  // Discrete kernel does not allow models to contain models.
  return 0;
}

int Session::findOrAddPrototype(
  const smtk::model::Instance& entRef,
  SessionInfoBits request,
  smtk::model::ArrangementHelper* helper)
{
  (void)request;
  (void)entRef;
  (void)helper;
  return 0;
}

int Session::findOrAddRelatedModels(
  const smtk::model::SessionRef& entRef,
  SessionInfoBits request,
  smtk::model::ArrangementHelper* helper)
{
  (void)request;
  (void)entRef;
  (void)helper;
  return 0;
}

int Session::findOrAddRelatedGroups(
  const smtk::model::EntityRef& entRef,
  SessionInfoBits request,
  smtk::model::ArrangementHelper* hlp)
{
  (void)request;
  ArrangementHelper* helper = dynamic_cast<ArrangementHelper*>(hlp);
  vtkDiscreteModelEntity* ent =
    dynamic_cast<vtkDiscreteModelEntity*>(
      this->entityForUUID(entRef.entity()));
  if (!ent)
    return 0;

  int numGroups = this->addEntities(
    ent->NewModelEntityGroupIterator(), entRef,
    smtk::model::SUPERSET_OF, helper);
  return numGroups;
}

int Session::findOrAddRelatedInstances(
  const smtk::model::EntityRef& entRef,
  SessionInfoBits request,
  smtk::model::ArrangementHelper* helper)
{
  (void)request;
  (void)entRef;
  (void)helper;
  return 0;
}

smtk::model::SessionInfoBits Session::findOrAddArrangements(
  const smtk::model::EntityRef& entRef,
  Entity* entRec,
  SessionInfoBits flags,
  smtk::model::ArrangementHelper* helper)
{
  (void)entRef;
  (void)entRec;
  (void)flags;
  (void)helper;
  return 0;
}

SessionInfoBits Session::updateProperties(
  const smtk::model::EntityRef& entRef,
  Entity* entRec,
  SessionInfoBits flags,
  smtk::model::ArrangementHelper* helper)
{
  (void)entRec;
  (void)helper;
  smtk::model::EntityRef mutableRef(entRef);
  this->addProperties(mutableRef, this->entityForUUID(entRef.entity()));
  if (flags & smtk::model::SESSION_PROPERTIES)
    {
    if (this->addProperties(mutableRef, this->entityForUUID(entRef.entity())))
      return smtk::model::SESSION_PROPERTIES;
    }
  return 0;
}

SessionInfoBits Session::updateTessellation(
  const smtk::model::EntityRef& entRef,
  SessionInfoBits flags,
  smtk::model::ArrangementHelper* helper)
{
  (void)flags;
  (void)helper;
  // TODO: Test and/or Add generation number property to indicate last time
  //       tessellation was generated relative to vtkModelItem...
  if (flags & smtk::model::SESSION_TESSELLATION)
    {
    vtkModelGeometricEntity* item =
      dynamic_cast<vtkModelGeometricEntity*>(
        this->entityForUUID(entRef.entity()));
    if (item && this->addTessellation(entRef, item))
      return smtk::model::SESSION_TESSELLATION;
    }
  return 0;
}

smtk::common::UUID Session::trackModel(
  vtkDiscreteModelWrapper* mod,
  const std::string& url,
  smtk::model::ManagerPtr mgr)
{
  vtkDiscreteModel* dmod = mod->GetModel();
  if (!dmod)
    return smtk::common::UUID::null();

  // Add or obtain the model's UUID
  smtk::common::UUID mid = this->findOrSetEntityUUID(dmod);
  // Track the model (keep a strong reference to it)
  // by UUID as well as the inverse map for quick reference:
  this->m_modelIdsToRefs[mid] = mod;
  this->m_modelRefsToIds[mod] = mid;
  this->m_itemsToRefs[mid] = dmod;
  this->m_modelsToSessions[dmod] = shared_from_this();
  smtk::model::Model smtkModel(mgr, mid);
  smtkModel.setSession(
    smtk::model::SessionRef(
      mgr, this->sessionId()));

  // Now add the record to manager and assign the URL to
  // the model as a string property.
  //smtk::model::EntityRef c = this->addCMBEntityToManager(mid, dmod, mgr, 8);
  smtk::model::EntityRef c(mgr, mid);
  this->declareDanglingEntity(c);
  this->transcribe(c, smtk::model::SESSION_EVERYTHING);
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
  smtk::common::UUID result = this->findOrSetEntityUUID(mp);
  if (result && item)
    this->m_itemsToRefs[result] = item;
  return result;
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

/// Add a child to \a parent's manager and a parent-child relationship to \a helper.
void Session::addEntity(
  const smtk::model::EntityRef& parent,
  vtkModelItem* child,
  smtk::model::ArrangementKind k,
  ArrangementHelper* helper,
  int sense,
  smtk::model::Orientation orientation)
{
  if(!child)
    {
    return;
    }
  smtk::model::EntityRef childRef(
    parent.manager(),
    this->findOrSetEntityUUID(child));
  this->addEntityRecord(parent);
  if (parent.isUseEntity()) helper->reset(parent);
  this->addEntityRecord(childRef);
  if (childRef.isUseEntity()) helper->reset(childRef);
  this->findOrAddRelatedEntities(childRef, smtk::model::SESSION_EVERYTHING, helper);
  helper->addArrangement(parent, k, childRef, sense, orientation);
}

/// Add a parent to the manager and a parent-child relationship to the helper.
void Session::addEntity(
  vtkModelItem* parent,
  const smtk::model::EntityRef& child,
  smtk::model::ArrangementKind k,
  ArrangementHelper* helper,
  int sense,
  smtk::model::Orientation orientation)
{
  if(!parent)
    {
    return;
    }
  smtk::model::EntityRef parentRef(
    child.manager(),
    this->findOrSetEntityUUID(parent));
  this->addEntityRecord(parentRef);
  if (parentRef.isUseEntity()) helper->reset(parentRef);
  this->addEntityRecord(child);
  if (child.isUseEntity()) helper->reset(child);
  this->findOrAddRelatedEntities(parentRef, smtk::model::SESSION_EVERYTHING, helper);
  helper->addArrangement(parentRef, k, child, sense, orientation);
}

/**\brief Add children to \a parent's manager and parent-child relationships to \a helper.
  *
  * The parent-child relationships map \a parent to each entry of \a it with the given type \a k.
  */
int Session::addEntities(
  const EntityRef& parent,
  vtkModelItemIterator* it,
  ArrangementKind k,
  ArrangementHelper* helper)
{
  int numEnts = 0;
  for (it->Begin(); !it->IsAtEnd(); it->Next(), ++numEnts)
    {
    this->addEntity(parent, it->GetCurrentItem(), k, helper, -1, smtk::model::UNDEFINED);
    }
  it->Delete();
  return numEnts;
}

/**\brief Add children to \a parent's manager and parent-child relationships to \a helper.
  *
  * The parent-child relationships map \a parent to each entry of \a it with the given type \a k.
  * This variant accepts a functor (\a senseLookup) which is invoked to determine the sense
  * and orientation of each item so that the arrangement is configured properly.
  */
template<typename T>
int Session::addEntities(
  const smtk::model::EntityRef& parent,
  vtkModelItemIterator* it,
  smtk::model::ArrangementKind k,
  ArrangementHelper* helper,
  T& senseLookup)
{
  int numEnts = 0;
  for (it->Begin(); !it->IsAtEnd(); it->Next(), ++numEnts)
    {
    int sense = -1;
    Orientation orientation = smtk::model::UNDEFINED;
    senseLookup(it->GetCurrentItem(), sense, orientation);
    this->addEntity(parent, it->GetCurrentItem(), k, helper, sense, orientation);
    }
  it->Delete();
  return numEnts;
}

/**\brief Add multiple parents to \a child's manager and parent-child relationships to \a helper.
  *
  * The parent-child relationships map \a parent to each entry of \a it with the given type \a k.
  */
int Session::addEntities(
  vtkModelItemIterator* it,
  const smtk::model::EntityRef& child,
  smtk::model::ArrangementKind k,
  ArrangementHelper* helper)
{
  int numEnts = 0;
  for (it->Begin(); !it->IsAtEnd(); it->Next(), ++numEnts)
    {
    this->addEntity(it->GetCurrentItem(), child, k, helper, -1, smtk::model::UNDEFINED);
    }
  it->Delete();
  return numEnts;
}

/**\brief Add multiple parents to \a child's manager and parent-child relationships to \a helper.
  *
  * The parent-child relationships map \a parent to each entry of \a it with the given type \a k.
  * This variant accepts a functor (\a senseLookup) which is invoked to determine the sense
  * and orientation of each item so that the arrangement is configured properly.
  */
template<typename T>
int Session::addEntities(
  vtkModelItemIterator* it,
  const smtk::model::EntityRef& child,
  smtk::model::ArrangementKind k,
  ArrangementHelper* helper,
  T& senseLookup)
{
  int numEnts = 0;
  for (it->Begin(); !it->IsAtEnd(); it->Next(), ++numEnts)
    {
    int sense;
    Orientation orientation;
    senseLookup(it->GetCurrentItem(), sense, orientation);
    this->addEntity(it->GetCurrentItem(), child, k, helper, sense, orientation);
    }
  it->Delete();
  return numEnts;
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
      cellOut.setIntegerProperty("visible", erec->GetVisibility());
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
  if (cellOut.isModel())
    {
    cellOut.setIntegerProperty(SMTK_GEOM_STYLE_PROP, smtk::model::DISCRETE);
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
        { // Add boundary groups to model
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
      if (material->GetNumberOfAssociations(vtkModelRegionType))
        { // Add regions to material
        this->addEntities(result, material->NewIterator(vtkModelRegionType), AddEntityToGroupHelper(), relDepth - 1);
        }
      else if(material->GetNumberOfAssociations(vtkModelFaceType))
        { // Add faces to material group
        this->addEntities(result, material->NewIterator(vtkModelFaceType), AddEntityToGroupHelper(), relDepth - 1);
        }
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
          loopUseIt->Delete();
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
  if (refVolume)
    {
    smtk::model::Volume result;
    // if there is a Volume already for refVolume, return it; otherwise, create one
    if(mgr->findEntity(uid, false))
      result = smtk::model::Volume(mgr, uid);
    else
      {
      result = mgr->insertVolume(uid);
      if (relDepth >= 0)
        {
        // Add refVolume relations and arrangements
        this->addEntities(result, refVolume->NewAdjacentModelFaceIterator(), AddRawRelationHelper(), relDepth - 1);
        this->addEntities(result, refVolume->NewIterator(vtkModelShellUseType), AddVolumeUseToVolumeHelper(), relDepth - 1);
        }
      this->addProperties(result, refVolume);
      }

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

// This will remove Model from smtk manager and vtkDiscreteModelWrapper form kernel
bool Session::removeModelEntity(const smtk::model::EntityRef& modRef)
{
  vtkDiscreteModelWrapper* modelWrap = this->findModelEntity(modRef.entity());
  if(!modelWrap)
    return false;
  vtkDiscreteModel* dmod = modelWrap->GetModel();

  // Remove any observers on model items that this session added:
  std::map<smtk::common::UUID, vtkWeakPointer<vtkModelItem> >::iterator mit;
  for (mit = this->m_itemsToRefs.begin(); mit != this->m_itemsToRefs.end(); ++mit)
    {
    if(!mit->second || mit->second.Get() != dmod)
      continue;
    vtkInformation* mp = mit->second->GetProperties();
    mp->RemoveObserver(this->m_itemWatcher);
    }

  // Remove any models that this session owned from s_modelsToSessions.
  std::map<vtkDiscreteModel*,Session::WeakPtr>::iterator mbit;
  for (mbit = this->m_modelsToSessions.begin(); mbit != this->m_modelsToSessions.end(); ++mbit)
    {
    if (mbit->first == dmod)
      {
      smtk::common::UUID modelId = modRef.entity();
      this->m_modelsToSessions.erase(mbit);
      vtkSmartPointer<vtkDiscreteModelWrapper> modelPtr = this->m_modelIdsToRefs[modelId];
      this->m_modelIdsToRefs.erase(modelId);
      this->m_modelRefsToIds.erase(modelPtr);
      break;
      }
    }

  return this->manager()->eraseModel(modRef);
}

void Session::retranscribeModel(const smtk::model::Model& inModel)
{
  smtk::common::UUID mid = inModel.entity();
  smtk::model::StringList const& urlprop(inModel.stringProperty("url"));
  std::string url;
  if (!urlprop.empty())
    {
    url = urlprop[0];
    }

  //FIXME. The group info seems to get lost somehow. The transcribe did not
  // bring back the groups defined in the model.
  // Need some special handling of groups
  Groups groups = inModel.groups();
  smtk::common::UUIDs grpids;
  for(Groups::const_iterator it=groups.begin(); it!=groups.end(); ++it)
      grpids.insert(it->entity());

  this->manager()->eraseModel(inModel);
  this->transcribe(inModel, smtk::model::SESSION_EVERYTHING, false);

  smtk::model::Model smtkModel(this->manager(), mid);
  smtk::model::SessionRef sess(
      this->manager(), this->sessionId());
  smtkModel.setSession(sess);

  // See above FIXME comments
  for(smtk::common::UUIDs::const_iterator it=grpids.begin(); it!=grpids.end(); ++it)
    {
    vtkModelItem* cmbgroup = this->entityForUUID(*it);
    if(cmbgroup)
      {
      smtk::model::Group smtkgroup = this->addCMBEntityToManager(*it, cmbgroup, this->manager());
      if(smtkgroup.isValid())
        smtkModel.addGroup(smtkgroup);
      }
    }

  if (!url.empty())
    {
    smtkModel.setStringProperty("url", url);
    }
}

smtk::bridge::discrete::BathymetryHelper* Session::bathymetryHelper()
{
  return this->m_bathymetryHelper;
}

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#include "smtk/bridge/discrete/Session_json.h"
smtkImplementsModelingKernel(
  SMTKDISCRETESESSION_EXPORT,
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
