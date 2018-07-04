//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_vtk_Resource_h
#define __smtk_session_vtk_Resource_h

#include "smtk/bridge/vtk/Exports.h"
#include "smtk/bridge/vtk/Session.h"

#include "smtk/model/Manager.h"

#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Manager.h"

namespace smtk
{
namespace bridge
{
namespace vtk
{

class SMTKVTKSESSION_EXPORT Resource
  : public smtk::resource::DerivedFrom<Resource, smtk::model::Manager>
{
public:
  smtkTypeMacro(smtk::bridge::vtk::Resource);
  smtkSharedPtrCreateMacro(smtk::resource::Resource);

  virtual ~Resource() {}

  const Session::Ptr& session() const { return m_session; }
  void setSession(const Session::Ptr&);

protected:
  Resource(const smtk::common::UUID&, smtk::resource::Manager::Ptr manager = nullptr);
  Resource(smtk::resource::Manager::Ptr manager = nullptr);

  Session::Ptr m_session;
};

} // namespace vtk
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_vtk_Resource_h
