//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_multiscale_Resource_h
#define __smtk_session_multiscale_Resource_h

#include "smtk/session/multiscale/Exports.h"
#include "smtk/session/multiscale/Session.h"

#include "smtk/session/mesh/Resource.h"

#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
namespace session
{
namespace multiscale
{

class SMTKMULTISCALESESSION_EXPORT Resource
  : public smtk::resource::DerivedFrom<Resource, smtk::session::mesh::Resource>
{
public:
  smtkTypeMacro(smtk::session::multiscale::Resource);
  smtkSharedPtrCreateMacro(smtk::resource::Resource);

  virtual ~Resource() {}

  Session::Ptr session() const
  {
    return std::static_pointer_cast<smtk::session::multiscale::Session>(
      smtk::session::mesh::Resource::session());
  }

  void setSession(const Session::Ptr& session)
  {
    return smtk::session::mesh::Resource::setSession(
      std::static_pointer_cast<smtk::session::mesh::Session>(session));
  }

protected:
  Resource(const smtk::common::UUID&, smtk::resource::Manager::Ptr manager = nullptr);
  Resource(smtk::resource::Manager::Ptr manager = nullptr);
};
}
}
}

#endif
