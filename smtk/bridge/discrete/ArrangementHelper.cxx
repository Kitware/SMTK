//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/discrete/ArrangementHelper.h"

#include "smtk/bridge/discrete/Session.h"

#include "smtk/model/ArrangementKind.h"
#include "smtk/model/Manager.h"

#include "smtk/common/UUIDGenerator.h"

#include "vtkModelEdge.h"
#include "vtkModelEdgeUse.h"

static smtk::common::UUIDGenerator s_idGen;

namespace {
  struct IterationOrder
  {
    template<typename T>
    bool operator()(const T& a, const T& b) const
    {
    return (a->kind < b->kind || (a->kind == b->kind &&
           (a->parent < b->parent || (a->parent == b->parent &&
           (a->iter_pos < b->iter_pos || (a->iter_pos == b->iter_pos &&
           (a->child < b->child || (a->child == b->child &&
            a->sense < b->sense)))))))) ? true : false;
    }
  };
}
namespace smtk {
  namespace bridge {
    namespace discrete {

/// Private constructor since this class is a base class which should not be instantiated.
ArrangementHelper::ArrangementHelper()
{
}

ArrangementHelper::~ArrangementHelper()
{
}

void ArrangementHelper::addArrangement(
  const smtk::model::EntityRef& parent,
  smtk::model::ArrangementKind k,
  const smtk::model::EntityRef& child)
{
  this->addArrangement(parent, k, child, -1, smtk::model::UNDEFINED, 0);
}

void ArrangementHelper::addArrangement(
  const smtk::model::EntityRef& parent,
  smtk::model::ArrangementKind k,
  const smtk::model::EntityRef& child,
  int sense,
  smtk::model::Orientation orientation,
  int iter_pos)
{

  Spec s(
    parent,
    child,
    k,
    2 * sense + (orientation == smtk::model::POSITIVE ? 1 : 0),
    iter_pos
    );

  // std::cout
  //   << "##Add " << parent.name() << " -"
  //   << smtk::model::NameForArrangementKind(k) << "- "
  //   << child.name()
  //   << " s " << sense << " o " << orientation
  //   << " iter_pos: " << iter_pos << "\n";

  typedef std::set<Spec>::const_iterator iter;
  std::pair<iter, bool> insertedInfo = this->m_arrangements.insert(s);
  if(insertedInfo.second == false)
    { //already added just update the iter_pos
    insertedInfo.first->iter_pos = iter_pos;
    }
}

void ArrangementHelper::resetArrangements()
{
  this->m_arrangements.clear();
  this->m_edgeUseSenses.clear();
  this->m_regionIds.clear();
}

/// This method is called after all related entities have been added and before arrangement updates are made.
void ArrangementHelper::doneAddingEntities(
  smtk::model::SessionPtr baseSession,
  smtk::model::SessionInfoBits flags)
{
  // I. Finish processing visited entities
  Session::Ptr sess = smtk::dynamic_pointer_cast<Session>(baseSession);
  smtk::model::EntityRefs::const_iterator eit;
  if (flags & smtk::model::SESSION_PROPERTIES)
    {
    for (eit = this->m_marked.begin(); eit != this->m_marked.end(); ++eit)
      {
      smtk::model::EntityRef mutableRef(*eit);
      vtkModelItem* dscEntity = sess->entityForUUID(eit->entity());
      sess->addProperties(mutableRef, dscEntity);
      }
    }

  // II. sort the specification based on IterationOrder. We use a set of
  //     pointers so we don't require the Specificaitions twice in memory
  //
  typedef std::set<Spec, IterationOrder>::iterator SpecIterType;
  std::set<const Spec*, IterationOrder> arrangement_inOrder;
  for (SpecIterType it = m_arrangements.begin(); it != m_arrangements.end(); ++it)
    {
    const Spec& s = *it;
    arrangement_inOrder.insert(&s);
    }

  // III. Add relations between visited entities
  //      Track groups and the entities they own; we need to add
  //      the group to the parent model. But we can only be guaranteed
  //      to find the parent model after all the arrangements have been
  //      processed. So, hold onto them in groupToMember.
  typedef std::set<const Spec*, IterationOrder>::iterator SpecPIterType;
  std::map<smtk::model::EntityRef, smtk::model::EntityRef> groupToMember;
  for (SpecPIterType itp = arrangement_inOrder.begin(); itp != arrangement_inOrder.end(); ++itp)
    {
    const Spec* it = *(itp);
    // std::cout
    //   << "Add " << it->parent.flagSummary(0) << " (" << it->parent.name() << ")"
    //   << " " << smtk::model::NameForArrangementKind(it->kind)
    //   << " " << it->child.flagSummary(0) << " (" << it->child.name() << ")"
    //   << " sense " << it->sense
    //   << " iter_pos " << it->iter_pos << "\n";

    if (it->parent.manager() != it->child.manager())
      {
      std::cerr << "  Mismatched or nil managers. Skipping.\n";
      }
    it->parent.manager()->addDualArrangement(
      it->parent.entity(), it->child.entity(),
      it->kind,
      /* sense */ it->sense / 2,
      it->sense % 2 ? smtk::model::POSITIVE : smtk::model::NEGATIVE);
    if (it->parent.isGroup())
      groupToMember[it->parent] = it->child;
    }
  // III. Find owning model for each group.
  std::map<smtk::model::EntityRef, smtk::model::EntityRef>::iterator git;
  smtk::model::Model owner;
  for (git = groupToMember.begin(); git != groupToMember.end(); ++git)
    if ((owner = git->second.owningModel()).isValid())
      git->first.manager()->addDualArrangement(
        owner.entity(), git->first.entity(), smtk::model::SUPERSET_OF,
        -1, smtk::model::UNDEFINED);

  // IV. Add tessellations for the entities.
  if (flags & smtk::model::SESSION_TESSELLATION)
    {
    for (eit = this->m_marked.begin(); eit != this->m_marked.end(); ++eit)
      {
      smtk::model::EntityRef mutableRef(*eit);
      vtkModelItem* dscEntity = sess->entityForUUID(eit->entity());
      vtkModelGeometricEntity* dscGeom =
        dynamic_cast<vtkModelGeometricEntity*>(dscEntity);
      if (dscGeom)
        {
        sess->addTessellation(mutableRef, dscGeom);
        }
      }
    }
}

int ArrangementHelper::findOrAssignSense(vtkModelEdgeUse* eu1)
{
  if (!eu1)
    return -1;
  vtkModelEdgeUse* eu2 = eu1->GetPairedModelEdgeUse();
  if (!eu2)
    return -1;
  vtkModelEdge* edge = eu1->GetModelEdge();
  EdgeToUseSenseMap::iterator eit = this->m_edgeUseSenses.find(edge);
  if (eit == this->m_edgeUseSenses.end())
    {
    EdgeUseToSenseMap entry;
    entry[eu1] = 0;
    entry[eu2] = 0;
    this->m_edgeUseSenses[edge] = entry;
    return 0;
    }
  EdgeUseToSenseMap::iterator sit = eit->second.find(eu1);
  if (sit == eit->second.end())
    {
    int nextSense = eit->second.size() / 2;
    eit->second[eu1] = nextSense;
    eit->second[eu2] = nextSense;
    return nextSense;
    }
  return sit->second;
}

template<typename T>
smtk::common::UUID IdForEntity(T* ent, std::map<T*,smtk::common::UUID>& fwd, std::map<smtk::common::UUID,T*>& bck)
{
  if (!ent)
    return smtk::common::UUID::null();

  typename std::map<T*,smtk::common::UUID>::const_iterator it = fwd.find(ent);
  if (it == fwd.end())
    {
    smtk::common::UUID regionId = s_idGen.random();
    fwd[ent] = regionId;
    bck[regionId] = ent;
    return regionId;
    }
  return it->second;
}

template<typename T>
T* EntityFromId(const smtk::common::UUID& entId, std::map<smtk::common::UUID,T*>& bck)
{
  typename std::map<smtk::common::UUID,T*>::const_iterator it = bck.find(entId);
  if (it == bck.end())
    return NULL;
  return it->second;
}

/// Given a discrete-model region (volume), return a UUID for it.
smtk::common::UUID ArrangementHelper::useForRegion(vtkModelRegion* region)
{
  return IdForEntity(region, this->m_regionIds, this->m_regions);
}

/// Given the UUID for a VolumeUse, return the discrete-model Region associated with it.
vtkModelRegion* ArrangementHelper::regionFromUseId(const smtk::common::UUID& volumeUseId)
{
  return EntityFromId(volumeUseId, this->m_regions);
}

/// Given a discrete-model edge-use, return a UUID for a chain bounding it.
smtk::common::UUID ArrangementHelper::chainForEdgeUse(vtkModelEdgeUse* edgeUse)
{
  return IdForEntity(edgeUse, this->m_chainIds, this->m_chains);
}

/// Given the UUID for a chain, return the discrete-model edge-use associated with it.
vtkModelEdgeUse* ArrangementHelper::edgeUseFromChainId(const smtk::common::UUID& chainId)
{
  return EntityFromId(chainId, this->m_chains);
}

    } // namespace discrete
  } // namespace bridge
} // namespace smtk
