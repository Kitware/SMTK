//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/SessionRef.h"

#include "smtk/attribute/Definition.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"

namespace smtk
{
namespace model
{

/**\brief Alternative constructor using session pointer instead of ID.
  *
  * This constructor obtains the session ID from the
  * \a brdg object.
  *
  * This variant also ensures that the session is registered
  * with the \a resource.
  */
SessionRef::SessionRef(ResourcePtr resource, SessionPtr brdg)
  : EntityRef(resource, brdg->sessionId())
{
  if (!!resource && !resource->sessionData(*this))
    resource->registerSession(brdg);
}

/**\brief Return the actual session this entityref references (or null).
  *
  */
Session::Ptr SessionRef::session() const
{
  ResourcePtr resource = m_resource.lock();
  if (!resource || !m_entity)
    return Session::Ptr();
  return resource->sessionData(*this);
}

/**\brief Add a model to the session.
  *
  * This adds a bijective arrangement to the session and the model.
  */
SessionRef& SessionRef::addModel(const Model& mod)
{
  this->addMemberEntity(mod);
  return *this;
}

/**\brief Return the session-class's tag data (a JSON string).
  *
  */
std::string SessionRef::tag() const
{
  // TODO: Session-specific information needs to be accessible without using
  // global statics.
  // return SessionRegistrar::sessionTags(this->session()->name());
  return std::string();
}

/**\brief Return the session-class's site name.
  *
  * This will be empty for local sessions.
  */
std::string SessionRef::site() const
{
  // TODO: Session-specific information needs to be accessible without using
  // global statics.
  // return SessionRegistrar::sessionSite(this->session()->name());
  return std::string();
}

/**\brief Return the session-class's list of engines.
  *
  * Engines are different modeling backends that can be used.
  * Often, an instance of a session can only support a single engine
  * even if support for several engines is available.
  */
StringList SessionRef::engines() const
{
  // TODO: Session-specific information needs to be accessible without using
  // global statics.
  // return SessionRegistrar::sessionEngines(this->session()->name());
  return StringList();
}

/**\brief Return the list of file types supported by this session.
  *
  * Passing in an engine name and context will return file types
  * specific to that \a engine in that \a context.
  *
  * Valid context strings are "read", "import", "write", and "export".
  * The default is "read".
  */
StringData SessionRef::fileTypes(const std::string& /*engine*/) const
{
  // TODO: Session-specific information needs to be accessible without using
  // global statics.
  // return SessionRegistrar::sessionFileTypes(this->session()->name(), engine);
  return StringData();
}

/// Return a filename extension (including ".") appropriate for saving \a model.
std::string SessionRef::defaultFileExtension(const Model& /*model*/) const
{
  // TODO: Session-specific information needs to be accessible without using
  // global statics.
  // return this->session()->defaultFileExtension(model);
  return std::string();
}

void SessionRef::close()
{
  ResourcePtr resource = this->resource();
  if (resource)
    resource->closeSession(*this);
}

/*! \fn template<typename T> T SessionRef::models() const;
  * \brief Return the list of models associated with this session.
  *
  * This returns all of the models for which Resource::setSessionForModel()
  * has been called with this entityref's session.
  */

} // namespace model
} // namespace smtk
