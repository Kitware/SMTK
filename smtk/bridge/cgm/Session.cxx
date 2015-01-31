//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/Session.h"
#include "smtk/bridge/cgm/Engines.h"
#include "smtk/bridge/cgm/TDUUID.h"

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

#include "smtk/common/UUID.h"

#include "smtk/Options.h"
#include "smtk/AutoInit.h"

#ifdef CGM_HAVE_VERSION_H
#  include "cgm_version.h"
#endif
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

#include "GMem.hpp"

typedef DLIList<RefEntity*> DLIRefList;

using smtk::model::EntityRef;
using namespace smtk::common;

namespace smtk {
  namespace bridge {
    namespace cgm {

/// Default constructor.
Session::Session()
{
  this->m_maxRelChordErr = 0.01; // fraction of longest edge.
  this->m_maxAngleErr = 2.0; // maximum angle in degrees.
  this->initializeOperatorSystem(Session::s_operators);
  if (!Engines::areInitialized())
    {
    Engines::isInitialized("");
    }
}

/// Virtual destructor. Here because Session overrides virtual methods from Session.
Session::~Session()
{
}

/// The CGM session supports smtk::model::SESSION_EVERYTHING.
smtk::model::SessionInfoBits Session::allSupportedInformation() const
{
  return smtk::model::SESSION_EVERYTHING;
}

/**\brief Create CGM entities for the given SMTK entities.
  *
  * This is not a complete implementation.
  */
bool Session::addManagerEntityToCGM(const smtk::model::EntityRef& ent)
{
  (void)ent;
  return true;
}

/**\brief Provide generic setup interface.
  *
  * Currently this only recognizes the "engine" keyword.
  * When passed \a optName "engine", the first entry in \a optVal
  * should be a valid modeling engine for CGM.
  */
int Session::staticSetup(const std::string& optName, const smtk::model::StringList& optVal)
{
  if (optName == "engine" && !optVal.empty())
    {
    return Engines::setDefault(optVal[0]) ? 1 : 0;
    }
  return 0;
}

/**\brief Accept post-construction configuration options.
  *
  * Currently CGM supports two options:
  * "tessellation maximum relative chord error" and
  * "tessellation maximum angle error".
  * Each accepts a single string that can be converted to
  * a floating point number.
  * If the value differs from the default (0.1 for the
  * chord error and 2 for the angle error), then a new
  * value is set and setup() returns 1. Otherwise 0 is
  * returned.
  */
int Session::setup(const std::string& optName, const smtk::model::StringList& optVal)
{
  char* valid;
  double value;
  if (!optVal.empty() && !optVal[0].empty())
    {
    if (optName == "tessellation maximum relative chord error")
      {
      value = strtod(optVal[0].c_str(), &valid);
      if (valid && value != this->m_maxRelChordErr)
        {
        this->m_maxRelChordErr = value;
        return 1;
        }
      }
    else if (optName == "tessellation maximum angle error")
      {
      value = strtod(optVal[0].c_str(), &valid);
      if (valid && value != this->m_maxAngleErr)
        {
        this->m_maxAngleErr = 2.0; // maximum angle in degrees.
        return 1;
        }
      }
    }
  return 0;
}

/**\brief Populate records for \a entityref that reflect the CGM \a entity.
  *
  */
smtk::model::SessionInfoBits Session::transcribeInternal(
  const smtk::model::EntityRef& entityref,
  SessionInfoBits requestedInfo)
{
  ToolDataUser* tdu = TDUUID::findEntityById(entityref.entity());
  RefEntity* ent = dynamic_cast<RefEntity*>(tdu);
  if (ent)
    return this->addCGMEntityToManager(entityref, ent, requestedInfo);
  /* Wishful thinking: GroupingEntity and SenseEntity do not inherit ToolDataUser
  GroupingEntity* grp = dynamic_cast<GroupingEntity*>(tdu);
  if (grp)
    return this->addCGMEntityToManager(entityref, grp, requestedInfo);
  SenseEntity* sns = dynamic_cast<SenseEntity*>(tdu);
  if (sns)
    return this->addCGMEntityToManager(entityref, sns, requestedInfo);
  */
  return 0;
}

smtk::model::SessionInfoBits Session::addCGMEntityToManager(
  const smtk::model::EntityRef& entityref, RefEntity* ent, SessionInfoBits requestedInfo)
{
  DagType dagType = ent->dag_type();
  if (dagType.is_valid())
    {
    switch (dagType.functional_type())
      {
    case DagType::BasicTopologyEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return this->addVolumeToManager(entityref, dynamic_cast<RefVolume*>(ent), requestedInfo);
      case 2: return this->addFaceToManager(entityref, dynamic_cast<RefFace*>(ent), requestedInfo);
      case 1: return this->addEdgeToManager(entityref, dynamic_cast<RefEdge*>(ent), requestedInfo);
      case 0: return this->addVertexToManager(entityref, dynamic_cast<RefVertex*>(ent), requestedInfo);
        }
      break;
    case DagType::SenseEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return this->addVolumeUseToManager(entityref, dynamic_cast<CoVolume*>(ent), requestedInfo);
      case 2: return this->addFaceUseToManager(entityref, dynamic_cast<CoFace*>(ent), requestedInfo);
      case 1: return this->addEdgeUseToManager(entityref, dynamic_cast<CoEdge*>(ent), requestedInfo);
      case 0: return this->addVertexUseToManager(entityref, dynamic_cast<CoVertex*>(ent), requestedInfo);
        }
      break;
    case DagType::GroupingEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return this->addBodyToManager(entityref, dynamic_cast<Body*>(ent), requestedInfo);
      case 2: return this->addShellToManager(entityref, dynamic_cast<Shell*>(ent), requestedInfo);
      case 1: return this->addLoopToManager(entityref, dynamic_cast<Loop*>(ent), requestedInfo);
      case 0: return this->addChainToManager(entityref, dynamic_cast<Chain*>(ent), requestedInfo);
        }
      break;
      }
    }
  // Might be a RefGroup
  RefGroup* grp = dynamic_cast<RefGroup*>(ent);
  if (grp) return this->addGroupToManager(entityref, grp, requestedInfo);

  // Nothing we know about
  std::cerr
    << "Invalid DagType"
    << "(" << dagType.dimension() << "," << dagType.functional_type() << ")"
    << " for entity \"" << ent->entity_name().c_str() << "\" " << ent << "\n";
  return 0;
}

smtk::model::SessionInfoBits Session::addCGMEntityToManager(
  const smtk::model::EntityRef& entityref, GroupingEntity* grp, SessionInfoBits requestedInfo)
{
  DagType dagType = grp->dag_type();
  if (dagType.is_valid())
    {
    switch (dagType.functional_type())
      {
    case DagType::BasicTopologyEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return this->addVolumeToManager(entityref, dynamic_cast<RefVolume*>(grp), requestedInfo);
      case 2: return this->addFaceToManager(entityref, dynamic_cast<RefFace*>(grp), requestedInfo);
      case 1: return this->addEdgeToManager(entityref, dynamic_cast<RefEdge*>(grp), requestedInfo);
      case 0: return this->addVertexToManager(entityref, dynamic_cast<RefVertex*>(grp), requestedInfo);
        }
      break;
    case DagType::SenseEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return this->addVolumeUseToManager(entityref, dynamic_cast<CoVolume*>(grp), requestedInfo);
      case 2: return this->addFaceUseToManager(entityref, dynamic_cast<CoFace*>(grp), requestedInfo);
      case 1: return this->addEdgeUseToManager(entityref, dynamic_cast<CoEdge*>(grp), requestedInfo);
      case 0: return this->addVertexUseToManager(entityref, dynamic_cast<CoVertex*>(grp), requestedInfo);
        }
      break;
    case DagType::GroupingEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return this->addBodyToManager(entityref, dynamic_cast<Body*>(grp), requestedInfo);
      case 2: return this->addShellToManager(entityref, dynamic_cast<Shell*>(grp), requestedInfo);
      case 1: return this->addLoopToManager(entityref, dynamic_cast<Loop*>(grp), requestedInfo);
      case 0: return this->addChainToManager(entityref, dynamic_cast<Chain*>(grp), requestedInfo);
        }
      break;
      }
    }
  // Might be a RefGroup
  RefGroup* refGroup = dynamic_cast<RefGroup*>(grp);
  if (refGroup) return this->addGroupToManager(entityref, refGroup, requestedInfo);

  // Nothing we know about
  std::cerr
    << "Invalid DagType"
    << "(" << dagType.dimension() << "," << dagType.functional_type() << ")"
    << " for entity " << grp << "\n";
  return 0;
}

smtk::model::SessionInfoBits Session::addCGMEntityToManager(
  const smtk::model::EntityRef& entityref, SenseEntity* sns, SessionInfoBits requestedInfo)
{
  DagType dagType = sns->dag_type();
  if (dagType.is_valid())
    {
    switch (dagType.functional_type())
      {
    case DagType::BasicTopologyEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return this->addVolumeToManager(entityref, dynamic_cast<RefVolume*>(sns), requestedInfo);
      case 2: return this->addFaceToManager(entityref, dynamic_cast<RefFace*>(sns), requestedInfo);
      case 1: return this->addEdgeToManager(entityref, dynamic_cast<RefEdge*>(sns), requestedInfo);
      case 0: return this->addVertexToManager(entityref, dynamic_cast<RefVertex*>(sns), requestedInfo);
        }
      break;
    case DagType::SenseEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return this->addVolumeUseToManager(entityref, dynamic_cast<CoVolume*>(sns), requestedInfo);
      case 2: return this->addFaceUseToManager(entityref, dynamic_cast<CoFace*>(sns), requestedInfo);
      case 1: return this->addEdgeUseToManager(entityref, dynamic_cast<CoEdge*>(sns), requestedInfo);
      case 0: return this->addVertexUseToManager(entityref, dynamic_cast<CoVertex*>(sns), requestedInfo);
        }
      break;
    case DagType::GroupingEntity_TYPE:
      switch (dagType.dimension())
        {
      case 3: return this->addBodyToManager(entityref, dynamic_cast<Body*>(sns), requestedInfo);
      case 2: return this->addShellToManager(entityref, dynamic_cast<Shell*>(sns), requestedInfo);
      case 1: return this->addLoopToManager(entityref, dynamic_cast<Loop*>(sns), requestedInfo);
      case 0: return this->addChainToManager(entityref, dynamic_cast<Chain*>(sns), requestedInfo);
        }
      break;
      }
    }
  // Might be a RefGroup
  RefGroup* grp = dynamic_cast<RefGroup*>(sns);
  if (grp) return this->addGroupToManager(entityref, grp, requestedInfo);

  // Nothing we know about
  std::cerr
    << "Invalid DagType"
    << "(" << dagType.dimension() << "," << dagType.functional_type() << ")"
    << " for entity " << sns << "\n";
  return 0;
}

/// Given a CGM \a body tagged with \a uid, create a record in \a modelManager for it.
smtk::model::SessionInfoBits Session::addBodyToManager(
  const smtk::model::Model& entityref,
  Body* body,
  SessionInfoBits requestedInfo)
{
  SessionInfoBits actual = 0;
  if (body)
    {
    EntityRef mutableEntityRef(entityref);
    mutableEntityRef.manager()->insertModel(entityref.entity(), 3, 3, body->entity_name().c_str());
    actual |= smtk::model::SESSION_ENTITY_TYPE;

    if (requestedInfo & (smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS))
      {
      // Add free cells, submodels, and groups.
      // Since CGM does not allow submodels, there's nothing to do for that.
      // However, we can see what groups *contain* this body.
      DLIList<RefEntity*> parents;
      body->get_parent_ref_entities(parents);
      // Free cells:
      DLIList<RefEntity*> children;
      body->get_child_ref_entities(children);
      //std::cout << "Body has " << parents.size() << " parents, " << children.size() << " children.\n";
      for (int i = 0; i < children.size(); ++i)
        {
        RefEntity* child = children.get_and_step();
        smtk::model::EntityRef childEntityRef(mutableEntityRef.manager(), TDUUID::ofEntity(child)->entityId());
        this->declareDanglingEntity(mutableEntityRef,
          this->transcribeInternal(childEntityRef, requestedInfo)); //smtk::model::SESSION_ENTITY_TYPE));
        //std::cout << "  " << child << "   " << childEntityRef.entity() << "\n";
        if (!childEntityRef.entity().isNull())
          {
          entityref.manager()->findOrAddInclusionToCellOrModel(
            entityref.entity(), childEntityRef.entity());
          }
        }
      actual |= smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS;
      }
    if (requestedInfo & smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS)
      {
      // FIXME: Todo.
      actual |= smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS;
      }
    if (requestedInfo & smtk::model::SESSION_TESSELLATION)
      {
      // FIXME: Todo.
      actual |= smtk::model::SESSION_TESSELLATION;
      }
    if (requestedInfo & smtk::model::SESSION_PROPERTIES)
      {
      // Set properties.
      this->addNamesIfAny(mutableEntityRef, body);
      // If the color is not the default color, add it as a property.
      this->colorPropFromIndex(mutableEntityRef, body->color());
      actual |= smtk::model::SESSION_PROPERTIES;
      }
    }
  return actual;
}

/// Given a CGM \a coVolume tagged with \a entityref's UUID, create a record in \a entityref's manager for it.
smtk::model::SessionInfoBits Session::addVolumeUseToManager(
  const smtk::model::VolumeUse& entityref,
  CoVolume* coVolume,
  SessionInfoBits requestedInfo)
{
  SessionInfoBits actual = 0;
  if (coVolume)
    {
    smtk::model::VolumeUse mutableEntityRef(entityref);
    mutableEntityRef.manager()->insertVolumeUse(entityref.entity());
    actual |= smtk::model::SESSION_ENTITY_TYPE;

    if (requestedInfo & (smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS))
      {
      // FIXME: Todo.
      actual |= smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS;
      }
    if (requestedInfo & smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS)
      {
      // FIXME: Todo.
      actual |= smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS;
      }
    if (requestedInfo & smtk::model::SESSION_TESSELLATION)
      {
      // FIXME: Todo.
      actual |= smtk::model::SESSION_TESSELLATION;
      }
    if (requestedInfo & smtk::model::SESSION_PROPERTIES)
      {
      // Set properties.
      // FIXME: Todo.
      }
    }
  return actual;
}

/// Given a CGM \a coFace tagged with \a uid, create a record in \a manager for it.
smtk::model::SessionInfoBits Session::addFaceUseToManager(
  const smtk::model::FaceUse& entityref,
  CoFace* coFace,
  SessionInfoBits requestedInfo)
{
  (void)entityref;
  SessionInfoBits actual = 0;
  if (coFace)
    {
    if (requestedInfo)
      {
      // Add coFace relations and arrangements
      }
    }
  return actual;
}

/// Given a CGM \a coEdge tagged with \a uid, create a record in \a manager for it.
smtk::model::SessionInfoBits Session::addEdgeUseToManager(
  const smtk::model::EdgeUse& entityref,
  CoEdge* coEdge,
  SessionInfoBits requestedInfo)
{
  (void)entityref;
  SessionInfoBits actual = 0;
  if (coEdge)
    {
    if (requestedInfo)
      {
      // Add coEdge relations and arrangements
      }
    }
  return actual;
}

/// Given a CGM \a coVertex tagged with \a uid, create a record in \a manager for it.
smtk::model::SessionInfoBits Session::addVertexUseToManager(
  const smtk::model::VertexUse& entityref,
  CoVertex* coVertex,
  SessionInfoBits requestedInfo)
{
  (void)entityref;
  SessionInfoBits actual = 0;
  if (coVertex)
    {
    if (requestedInfo)
      {
      // Add coVertex relations and arrangements
      }
    }
  return actual;
}

/// Given a CGM \a shell tagged with \a uid, create a record in \a manager for it.
smtk::model::SessionInfoBits Session::addShellToManager(
  const smtk::model::Shell& entityref,
  Shell* shell,
  SessionInfoBits requestedInfo)
{
  (void)entityref;
  SessionInfoBits actual = 0;
  if (shell)
    {
    if (requestedInfo)
      {
      // Add shell relations and arrangements
      }
    }
  return actual;
}

/// Given a CGM \a loop tagged with \a uid, create a record in \a manager for it.
smtk::model::SessionInfoBits Session::addLoopToManager(
  const smtk::model::Loop& entityref,
  Loop* loop,
  SessionInfoBits requestedInfo)
{
  (void)entityref;
  SessionInfoBits actual = 0;
  if (loop)
    {
    if (requestedInfo)
      {
      // Add loop relations and arrangements
      }
    }
  return actual;
}

/// Given a CGM \a chain tagged with \a uid, create a record in \a manager for it.
smtk::model::SessionInfoBits Session::addChainToManager(
  const smtk::model::Chain& entityref,
  Chain* chain,
  SessionInfoBits requestedInfo)
{
  (void)entityref;
  SessionInfoBits actual = 0;
  if (chain)
    {
    if (requestedInfo)
      {
      // Add chain relations and arrangements
      }
    }
  return actual;
}

/// Given a CGM \a refVolume tagged with \a uid, create a record in \a manager for it.
smtk::model::SessionInfoBits Session::addVolumeToManager(
  const smtk::model::Volume& entityref,
  RefVolume* refVolume,
  SessionInfoBits requestedInfo)
{
  (void)entityref;
  SessionInfoBits actual = 0;
  if (refVolume)
    {
    smtk::model::Volume mutableEntityRef(entityref);
    mutableEntityRef.manager()->insertVolume(entityref.entity());
    actual |= smtk::model::SESSION_ENTITY_TYPE;

    if (requestedInfo & (smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS))
      {
      // Add child relationships
      DLIList<RefEntity*> rels;
      refVolume->get_child_ref_entities(rels);
      this->addRelations(mutableEntityRef, rels, requestedInfo, 3);
      // Add parent relationships
      refVolume->get_parent_ref_entities(rels);
      this->addRelations(mutableEntityRef, rels, requestedInfo, -1);

      actual |= smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS;
      }
    if (requestedInfo & smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS)
      {
      // FIXME: Todo.
      actual |= smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS;
      }
    if (requestedInfo & smtk::model::SESSION_TESSELLATION)
      {
      // FIXME: Todo.
      actual |= smtk::model::SESSION_TESSELLATION;
      }
    if (requestedInfo & smtk::model::SESSION_PROPERTIES)
      {
      // Set properties.
      this->addNamesIfAny(mutableEntityRef, refVolume);
      // If the color is not the default color, add it as a property.
      this->colorPropFromIndex(mutableEntityRef, refVolume->color());
      actual |= smtk::model::SESSION_PROPERTIES;
      }
    }
  return actual;
}

/// Given a CGM \a refFace tagged with \a uid, create a record in \a manager for it.
smtk::model::SessionInfoBits Session::addFaceToManager(
  const smtk::model::Face& entityref,
  RefFace* refFace,
  SessionInfoBits requestedInfo)
{
  SessionInfoBits actual = 0;
  if (refFace)
    {
    smtk::model::Face mutableEntityRef(entityref);
    if (!mutableEntityRef.isValid())
      mutableEntityRef.manager()->insertFace(entityref.entity());
    actual |= smtk::model::SESSION_ENTITY_TYPE;

    if (requestedInfo & (smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS))
      {
      // Add child relationships
      DLIList<RefEntity*> rels;
      refFace->get_child_ref_entities(rels);
      this->addRelations(mutableEntityRef, rels, requestedInfo, 2);
      // Add parent relationships
      refFace->get_parent_ref_entities(rels);
      this->addRelations(mutableEntityRef, rels, requestedInfo, -1);

      actual |= smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS;
      }
    if (requestedInfo & smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS)
      {
      // FIXME: Todo.
      actual |= smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS;
      }
    if (requestedInfo & smtk::model::SESSION_TESSELLATION)
      {
      if (this->addTessellation(entityref, refFace))
        actual |= smtk::model::SESSION_TESSELLATION;
      }
    if (requestedInfo & smtk::model::SESSION_PROPERTIES)
      {
      // Set properties.
      this->addNamesIfAny(mutableEntityRef, refFace);
      // If the color is not the default color, add it as a property.
      this->colorPropFromIndex(mutableEntityRef, refFace->color());
      actual |= smtk::model::SESSION_PROPERTIES;
      }
    }
  return actual;
}

/// Given a CGM \a refEdge tagged with \a uid, create a record in \a manager for it.
smtk::model::SessionInfoBits Session::addEdgeToManager(
  const smtk::model::Edge& entityref,
  RefEdge* refEdge,
  SessionInfoBits requestedInfo)
{
  SessionInfoBits actual = 0;
  if (refEdge)
    {
    smtk::model::Edge mutableEntityRef(entityref);
    if (!mutableEntityRef.isValid())
      mutableEntityRef.manager()->insertEdge(entityref.entity());
    actual |= smtk::model::SESSION_ENTITY_TYPE;

    if (requestedInfo & (smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS))
      {
      // Add refEdge relations and arrangements
      // Add child relationships
      DLIList<RefEntity*> rels;
      refEdge->get_child_ref_entities(rels);
      this->addRelations(mutableEntityRef, rels, requestedInfo, 1);
      // Add parent relationships
      refEdge->get_parent_ref_entities(rels);
      this->addRelations(mutableEntityRef, rels, requestedInfo, -1);

      actual |= smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS;
      }
    if (requestedInfo & smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS)
      {
      // FIXME: Todo.
      actual |= smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS;
      }
    if (requestedInfo & smtk::model::SESSION_TESSELLATION)
      {
      if (this->addTessellation(entityref, refEdge))
        actual |= smtk::model::SESSION_TESSELLATION;
      }
    if (requestedInfo & smtk::model::SESSION_PROPERTIES)
      {
      // Set properties.
      this->addNamesIfAny(mutableEntityRef, refEdge);
      // If the color is not the default color, add it as a property.
      this->colorPropFromIndex(mutableEntityRef, refEdge->color());
      actual |= smtk::model::SESSION_PROPERTIES;
      }
    }
  return actual;
}

/// Given a CGM \a refVertex tagged with \a uid, create a record in \a manager for it.
smtk::model::SessionInfoBits Session::addVertexToManager(
  const smtk::model::Vertex& entityref,
  RefVertex* refVertex,
  SessionInfoBits requestedInfo)
{
  SessionInfoBits actual = 0;
  if (refVertex)
    {
    smtk::model::Vertex mutableEntityRef(entityref);
    if (!mutableEntityRef.isValid())
      mutableEntityRef.manager()->insertVertex(entityref.entity());
    actual |= smtk::model::SESSION_ENTITY_TYPE;

    if (requestedInfo & (smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS))
      {
      // Add refVertex relations and arrangements
      DLIList<RefEntity*> rels;
      // Add parent relationships
      refVertex->get_parent_ref_entities(rels);
      this->addRelations(mutableEntityRef, rels, requestedInfo, 0);

      actual |= smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS;
      }
    if (requestedInfo & smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS)
      {
      // FIXME: Todo.
      actual |= smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS;
      }
    if (requestedInfo & smtk::model::SESSION_TESSELLATION)
      {
      if (this->addTessellation(entityref, refVertex))
        actual |= smtk::model::SESSION_TESSELLATION;
      }
    if (requestedInfo & smtk::model::SESSION_PROPERTIES)
      {
      // Set properties.
      this->addNamesIfAny(mutableEntityRef, refVertex);
      // If the color is not the default color, add it as a property.
      this->colorPropFromIndex(mutableEntityRef, refVertex->color());
      actual |= smtk::model::SESSION_PROPERTIES;
      }
    }
  return actual;
}

/// Given a CGM \a refGroup tagged with \a uid, create a record in \a manager for it.
smtk::model::SessionInfoBits Session::addGroupToManager(
  const smtk::model::Group& entityref,
  RefGroup* refGroup,
  SessionInfoBits requestedInfo)
{
  (void)entityref;
  SessionInfoBits actual = 0;
  if (refGroup)
    {
    smtk::model::Group mutableEntityRef(entityref);
    if (!mutableEntityRef.isValid())
      mutableEntityRef.manager()->insertGroup(entityref.entity());
    actual |= smtk::model::SESSION_ENTITY_TYPE;

    if (requestedInfo & (smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS))
      {
      // Add child relationships
      DLIList<RefEntity*> rels;
      refGroup->get_child_ref_entities(rels);
      this->addRelations(mutableEntityRef, rels, requestedInfo, 1);
      // Add parent relationships
      refGroup->get_parent_ref_entities(rels);
      this->addRelations(mutableEntityRef, rels, requestedInfo, 1);

      actual |= smtk::model::SESSION_ENTITY_RELATIONS | smtk::model::SESSION_ARRANGEMENTS;
      }
    if (requestedInfo & smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS)
      {
      // FIXME: Todo.
      actual |= smtk::model::SESSION_ATTRIBUTE_ASSOCIATIONS;
      }
    if (requestedInfo & smtk::model::SESSION_TESSELLATION)
      {
      // FIXME: Will we ever do this?
      }
    if (requestedInfo & smtk::model::SESSION_PROPERTIES)
      {
      // Set properties.
      this->addNamesIfAny(mutableEntityRef, refGroup);
      // If the color is not the default color, add it as a property.
      this->colorPropFromIndex(mutableEntityRef, refGroup->color());
      actual |= smtk::model::SESSION_PROPERTIES;
      }
    }
  return actual;
}

void Session::addRelations(
  smtk::model::EntityRef& entityref,
  DLIList<RefEntity*>& rels,
  SessionInfoBits requestedInfo,
  int depth)
{
  int nc = rels.size();
  for (int j = 0; j < nc; ++j)
    {
    RefEntity* rel = rels.get_and_step();
    TDUUID* refId = smtk::bridge::cgm::TDUUID::ofEntity(rel, true);
    smtk::common::UUID smtkChildId = refId->entityId();
    EntityRef smtkChild(entityref.manager(), smtkChildId);
    entityref.findOrAddRawRelation(smtkChild);
    if (depth > 0)
      this->addCGMEntityToManager(smtkChild, rel, requestedInfo);
    }
}

template<typename E>
bool SessionAddTessellation(const EntityRef& entityref, E* cgmEnt, double chordErr, double angleErr)
{
  if (!cgmEnt || !entityref.manager() || !entityref.entity())
    return false;

  GMem primitives;
  double measure = cgmEnt->measure();
  double maxErr = pow(measure, 1./cgmEnt->dimension()) * chordErr;
  double longestEdge = measure;
  cgmEnt->get_graphics(primitives, angleErr, maxErr, longestEdge);
  int connCount = primitives.fListCount;
  int npts = primitives.pointListCount;
  if (npts <= 0 || (connCount <= 0 && cgmEnt->dimension() > 1))
    {
    return false;
    }
  smtk::model::Tessellation blank;
  smtk::model::UUIDsToTessellations& tess(entityref.manager()->tessellations());
  smtk::model::UUIDsToTessellations::iterator it =
    tess.insert(std::pair<smtk::common::UUID,smtk::model::Tessellation>(entityref.entity(), blank)).first;
  it->second.reset();
  // Now add data to the Tessellation "in situ" to avoid a copy.
  // First, copy point coordinates:
  it->second.coords().reserve(3 * npts);
  GPoint* inPts = primitives.point_list();
  for (int j = 0; j < npts; ++j, ++inPts)
    {
    it->second.addCoords(inPts->x, inPts->y, inPts->z);
    }
  // Now translate the connectivity:
  if (cgmEnt->dimension() > 1)
    {
    it->second.conn().reserve(connCount);
    int* inConn = primitives.facet_list();
    int ptsPerPrim = 0;
    for (int k = 0; k < connCount; k += (ptsPerPrim + 1), inConn += (ptsPerPrim + 1))
      {
      ptsPerPrim = *inConn;
      int* pConn = inConn + 1;
      switch (ptsPerPrim)
        {
      case 1:
        // This is a vertex
        it->second.addPoint(pConn[0]);
        break;
      case 2:
        it->second.addLine(pConn[0], pConn[1]);
        break;
      case 3:
        it->second.addTriangle(pConn[0], pConn[1], pConn[2]);
        break;
      case 4:
        it->second.addQuad(pConn[0], pConn[1], pConn[2], pConn[3]);
      default:
        std::cerr << "Unknown primitive has " << ptsPerPrim << " conn entries\n";
        break;
        }
      }
    }
  else
    {
    it->second.conn().reserve(npts + 2);
    it->second.conn().push_back(smtk::model::TESS_POLYLINE);
    it->second.conn().push_back(npts);
    for (int k = 0; k < npts; ++k)
      {
      it->second.conn().push_back(k);
      }
    }
  return true;
}

template<>
bool SessionAddTessellation(const EntityRef& entityref, RefVertex* cgmEnt, double, double)
{
  if (!cgmEnt || !entityref.manager() || !entityref.entity())
    return false;

  std::vector<int> cellConn;
  cellConn.push_back(smtk::model::TESS_VERTEX);
  cellConn.push_back(0);

  CubitVector coords = cgmEnt->coordinates();
  smtk::model::Tessellation blank;
  smtk::model::UUIDsToTessellations& tess(entityref.manager()->tessellations());
  smtk::model::UUIDsToTessellations::iterator it =
    tess.insert(
      std::pair<smtk::common::UUID,smtk::model::Tessellation>(
        entityref.entity(), blank)).first;
  // Now add data to the Tessellation "in situ" to avoid a copy.
  // First, copy point coordinates:
  it->second.coords().resize(3);
  coords.get_xyz(&it->second.coords()[0]);
  it->second.insertNextCell(cellConn);
  return true;
}

/**\brief Ask CGM to hand us a tessellation of the \a cgmEnt for rendering.
  *
  * The resulting tessellation is stored in the \a entityref's model manager
  * and accessible from the entityref.
  */
///@{
bool Session::addTessellation(const EntityRef& entityref, RefFace* cgmEnt)
{
  return SessionAddTessellation(entityref, cgmEnt, this->m_maxRelChordErr, this->m_maxAngleErr);
}

bool Session::addTessellation(const EntityRef& entityref, RefEdge* cgmEnt)
{
  return SessionAddTessellation(entityref, cgmEnt, this->m_maxRelChordErr, this->m_maxAngleErr);
}

bool Session::addTessellation(const EntityRef& entityref, RefVertex* cgmEnt)
{
  return SessionAddTessellation(entityref, cgmEnt, this->m_maxRelChordErr, this->m_maxAngleErr);
}
///@}

/**\brief Add names attached to \a cgmEnt to the SMTK entity \a entityref.
  *
  * Returns true if any names were attached to \a cgmEnt and
  * false otherwise.
  */
bool Session::addNamesIfAny(smtk::model::EntityRef& entityref, RefEntity* cgmEnt)
{
  int numNames = cgmEnt->num_names();
  if (numNames > 0)
    {
#if !defined(CGM_MAJOR_VERSION) || CGM_MAJOR_VERSION <= 13
    DLIList<CubitString*> cgmNames;
    cgmEnt->entity_names(cgmNames);
    smtk::model::StringList smtkNames;
    for (int nn = 0; nn < numNames; ++nn)
      {
      CubitString* name = cgmNames.get_and_step();
      smtkNames.push_back(name->c_str());
      }
#else // CGM_MAJOR_VERSION >= 14
    DLIList<CubitString> cgmNames;
    cgmEnt->entity_names(cgmNames);
    smtk::model::StringList smtkNames;
    for (int nn = 0; nn < numNames; ++nn)
      {
      CubitString name = cgmNames.get_and_step();
      smtkNames.push_back(name.c_str());
      }
#endif // CGM_MAJOR_VERSION
    entityref.setStringProperty("name", smtkNames);
    return true;
    }
  return false;
}

/**\brief Assign an RGBA color to \a uid base on \a colorIndex.
  *
  * Cubit only provides 17 colors. Wah.
  */
void Session::colorPropFromIndex(
  smtk::model::EntityRef& entityref, int colorIndex)
{
  if (colorIndex != CUBIT_DEFAULT_COLOR)
    {
    smtk::model::FloatList rgba(4);
    rgba[3] = 1.; // All CGM colors are solid, not transparent.
    switch (colorIndex)
      {
    case CUBIT_BLACK:       rgba[0] = 0.00; rgba[1] = 0.00; rgba[2] = 0.00; break;
    case CUBIT_GREY:        rgba[0] = 0.75; rgba[1] = 0.75; rgba[2] = 0.75; break;
    case CUBIT_ORANGE:      rgba[0] = 1.00; rgba[1] = 0.65; rgba[2] = 0.00; break;
    case CUBIT_RED:         rgba[0] = 0.86; rgba[1] = 0.08; rgba[2] = 0.24; break;
    case CUBIT_GREEN:       rgba[0] = 0.00; rgba[1] = 0.50; rgba[2] = 0.00; break;
    case CUBIT_YELLOW:      rgba[0] = 1.00; rgba[1] = 1.00; rgba[2] = 0.00; break;
    case CUBIT_MAGENTA:     rgba[0] = 0.50; rgba[1] = 0.00; rgba[2] = 0.50; break;
    case CUBIT_CYAN:        rgba[0] = 0.00; rgba[1] = 0.50; rgba[2] = 0.50; break;
    case CUBIT_BLUE:        rgba[0] = 0.00; rgba[1] = 0.00; rgba[2] = 1.00; break;
    case CUBIT_WHITE:       rgba[0] = 1.00; rgba[1] = 1.00; rgba[2] = 1.00; break;
    case CUBIT_BROWN:       rgba[0] = 0.55; rgba[1] = 0.27; rgba[2] = 0.07; break;
    case CUBIT_GOLD:        rgba[0] = 1.00; rgba[1] = 0.84; rgba[2] = 0.00; break;
    case CUBIT_LIGHTBLUE:   rgba[0] = 0.88; rgba[1] = 1.00; rgba[2] = 1.00; break;
    case CUBIT_LIGHTGREEN:  rgba[0] = 0.56; rgba[1] = 0.93; rgba[2] = 0.56; break;
    case CUBIT_SALMON:      rgba[0] = 0.98; rgba[1] = 0.50; rgba[2] = 0.45; break;
    case CUBIT_CORAL:       rgba[0] = 0.86; rgba[1] = 0.44; rgba[2] = 0.58; break;
    case CUBIT_PINK:        rgba[0] = 1.00; rgba[1] = 0.71; rgba[2] = 0.76; break;
    default: rgba[3] = -1.; // invalid.
      }
    if (rgba[3] >= 0.)
      entityref.setFloatProperty("color", rgba);
    }
}

    } // namespace cgm
  } //namespace bridge
} // namespace smtk

#include "smtk/bridge/cgm/Session_json.h" // For Session_json
smtkImplementsModelingKernel(
  cgm,
  Session_json,
  smtk::bridge::cgm::Session::staticSetup,
  smtk::bridge::cgm::Session,
  true /* inherit "universal" operators */
);
