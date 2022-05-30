//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/polygon/Session.h"
#include "smtk/session/polygon/SessionIOJSON.h"

#include "smtk/AutoInit.h"

#include "smtk/model/Chain.h"
#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Group.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/SessionRef.h"
#include "smtk/model/Shell.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/VertexUse.h"
#include "smtk/model/Volume.h"
#include "smtk/model/VolumeUse.h"

#include "smtk/common/UUID.h"

#include "smtk/Options.h"
#include "smtk/attribute/Definition.h"
#include "smtk/session/polygon/internal/Model.h"
#include "smtk/session/polygon/internal/Vertex.h"

#include <cstring> // for strcmp

using namespace smtk::common;

namespace smtk
{
namespace session
{
namespace polygon
{

/// Default constructor.
Session::Session() = default;

/// Virtual destructor. Here because Session overrides virtual methods from Session.
Session::~Session() = default;

/// The polygon session supports smtk::model::SESSION_EVERYTHING.
smtk::model::SessionInfoBits Session::allSupportedInformation() const
{
  return smtk::model::SESSION_EVERYTHING;
}

std::string Session::defaultFileExtension(const smtk::model::Model& /*model*/) const
{
  return "";
}

smtk::model::SessionInfoBits Session::transcribeInternal(
  const smtk::model::EntityRef& entity,
  smtk::model::SessionInfoBits requestedInfo,
  int depth)
{
  if (entity.isModel())
  {
    smtk::model::EntityRef mutableEnt(entity);
    mutableEnt.setIntegerProperty(SMTK_GEOM_STYLE_PROP, smtk::model::DISCRETE);
  }

  (void)requestedInfo;
  (void)depth;
  return smtk::model::SESSION_EVERYTHING;
}

void Session::addStorage(
  const smtk::common::UUID& uid,
  smtk::session::polygon::internal::entity::Ptr s)
{
  m_storage[uid] = s;
}

bool Session::removeStorage(const smtk::common::UUID& uid)
{
  return m_storage.erase(uid) > 0;
}

/**\brief Remove all references to \a face from the polygon-session internal storage.
  *
  * Note that this must be called **before** \a face is removed from the SMTK model resource
  * since it uses information in the model resource to obtain the list of vertices bounding
  * the face.
  */
bool Session::removeFaceReferences(const smtk::model::Face& face)
{
  bool ok = true;
  // Find all vertices of face.
  smtk::model::Edges fe = face.edges();
  smtk::model::VertexSet fv;
  smtk::model::Edges::iterator eit;
  for (eit = fe.begin(); eit != fe.end(); ++eit)
  {
    smtk::model::Vertices ev = eit->vertices();
    fv.insert(ev.begin(), ev.end());
  }
  for (smtk::model::VertexSet::iterator vit = fv.begin(); vit != fv.end(); ++vit)
  {
    internal::vertex::Ptr vrec = this->findStorage<internal::vertex>(vit->entity());
    // If removeFaceAdjacencies returns 0, it means the face was not
    // listed at that vertex. That is not OK:
    if (!vrec || vrec->removeFaceAdjacencies(face.entity()) <= 0)
    {
      smtkErrorMacro(
        this->log(),
        "Face " << face.name() << " (" << face.entity() << ") "
                << " not bounded by " << vit->name() << " (" << vit->entity() << ")");
      ok = false;
    }
  }
  return ok;
}

/**\brief Remove all references to \a edge from the polygon-session internal storage.
  *
  * Note that this must be called **before** \a edge is removed from the SMTK model resource
  * since it uses information in the model resource to obtain the list of vertices bounding
  * the edge.
  */
bool Session::removeEdgeReferences(const smtk::model::Edge& edge)
{
  smtk::model::Vertices ev = edge.vertices();
  for (smtk::model::Vertices::iterator vit = ev.begin(); vit != ev.end(); ++vit)
  {
    internal::vertex::Ptr vrec = this->findStorage<internal::vertex>(vit->entity());
    // If removeIncidentEdge returns 0, it means the edge was not
    // incident to a vertex supposedly bounding it. That is not OK:
    if (!vrec || vrec->removeIncidentEdge(edge.entity()) <= 0)
    {
      std::cerr << "Edge " << edge.name() << " not incident to " << vit->name() << "\n";
      return false;
    }
  }
  return this->removeStorage(edge.entity());
}

/**\brief Remove all references to \a v from the polygon-session internal storage.
  *
  * This will emit an error and return false if the vertex has any incident edges.
  * It is otherwise identical to calling removeStorage(\a v .entity()).
  */
bool Session::removeVertReferences(const smtk::model::Vertex& v)
{
  internal::vertex::Ptr vrec = this->findStorage<internal::vertex>(v.entity());
  if (!vrec || vrec->edgesBegin() != vrec->edgesEnd())
  {
    smtkWarningMacro(
      this->log(),
      "Could not remove vertex " << v.name() << " because it is invalid or has incident edges.");
    return false;
  }
  internal::pmodel* pmod = vrec->parentAs<internal::pmodel>();
  if (pmod)
  {
    pmod->removeVertexLookup(vrec->point(), v.entity());
  }
  return this->removeStorage(v.entity());
}

smtk::model::SessionIOPtr Session::createIODelegate(const std::string& format)
{
  if (format == "json")
  {
    return SessionIOJSON::create();
  }
  return nullptr;
}

internal::EntityIdToPtr::iterator Session::findStorageIterator(const smtk::common::UUID& uid)
{
  return m_storage.find(uid);
}

internal::EntityIdToPtr::iterator Session::beginStorage()
{
  return m_storage.begin();
}

internal::EntityIdToPtr::iterator Session::endStorage()
{
  return m_storage.end();
}

internal::EntityIdToPtr::const_iterator Session::beginStorage() const
{
  return m_storage.begin();
}

internal::EntityIdToPtr::const_iterator Session::endStorage() const
{
  return m_storage.end();
}

} // namespace polygon
} //namespace session
} // namespace smtk
