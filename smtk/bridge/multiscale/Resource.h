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

#include "smtk/bridge/multiscale/Exports.h"
#include "smtk/bridge/multiscale/Session.h"

#include "smtk/bridge/mesh/Resource.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
namespace bridge
{
namespace multiscale
{

class SMTKMULTISCALESESSION_EXPORT Resource : public smtk::bridge::mesh::Resource
{
public:
  smtkTypeMacro(smtk::bridge::multiscale::Resource);
  smtkSharedPtrCreateMacro(smtk::resource::Resource);
  smtkResourceTypeNameMacro("multiscale model");

  // typedef referring to the parent resource.
  typedef smtk::bridge::mesh::Resource ParentResource;

  virtual ~Resource() {}

  Session::Ptr session() const
  {
    return std::static_pointer_cast<smtk::bridge::multiscale::Session>(
      smtk::bridge::mesh::Resource::session());
  }

  void setSession(const Session::Ptr& session)
  {
    return smtk::bridge::mesh::Resource::setSession(
      std::static_pointer_cast<smtk::bridge::mesh::Session>(session));
  }

protected:
  Resource(const smtk::common::UUID&, smtk::resource::Manager::Ptr manager = nullptr);
  Resource(smtk::resource::Manager::Ptr manager = nullptr);
};
}
}
}

#endif
