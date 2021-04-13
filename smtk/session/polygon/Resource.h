//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_polygon_Resource_h
#define __smtk_session_polygon_Resource_h

#include "smtk/session/polygon/Exports.h"
#include "smtk/session/polygon/Session.h"

#include "smtk/resource/DerivedFrom.h"

#include "smtk/model/Resource.h"

#include "smtk/resource/Manager.h"

namespace smtk
{
namespace session
{
namespace polygon
{

class Session;
typedef smtk::shared_ptr<Session> SessionPtr;
typedef smtk::shared_ptr<const Session> ConstSessionPtr;

class SMTKPOLYGONSESSION_EXPORT Resource
  : public smtk::resource::DerivedFrom<Resource, smtk::model::Resource>
{
public:
  smtkTypeMacro(smtk::session::polygon::Resource);
  smtkSharedPtrCreateMacro(smtk::resource::PersistentObject);

  virtual ~Resource() {}

  void setSession(const Session::Ptr&);

  SessionPtr polygonSession();
  ConstSessionPtr polygonSession() const;

  void addStorage(
    const smtk::common::UUID& uid,
    smtk::session::polygon::internal::entity::Ptr storage);
  bool removeStorage(const smtk::common::UUID& uid);
  template<typename T>
  typename T::Ptr findStorage(const smtk::common::UUID& uid)
  {
    return this->polygonSession()->findStorage<T>(uid);
  }
  template<typename T>
  T findOrAddStorage(const smtk::common::UUID& uid)
  {
    return this->polygonSession()->findOrAddStorage<T>(uid);
  }
  int nextModelNumber() { return this->polygonSession()->m_nextModelNumber++; }

protected:
  Resource(const smtk::common::UUID&, smtk::resource::ManagerPtr manager = nullptr);
  Resource(smtk::resource::ManagerPtr manager = nullptr);

  Session::Ptr m_session;
};

} // namespace polygon
} // namespace session
} // namespace smtk

#endif // __smtk_session_polygon_Resource_h
