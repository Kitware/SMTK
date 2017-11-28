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

#include "smtk/bridge/polygon/Exports.h"
#include "smtk/bridge/polygon/Session.h"

#include "smtk/model/Manager.h"

#include "smtk/resource/Manager.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{

class Session;
typedef smtk::shared_ptr<Session> SessionPtr;
typedef smtk::shared_ptr<const Session> ConstSessionPtr;

class SMTKPOLYGONSESSION_EXPORT Resource : public smtk::model::Manager
{
public:
  smtkTypeMacro(smtk::bridge::polygon::Resource);
  smtkSharedPtrCreateMacro(smtk::resource::Resource);
  smtkResourceTypeNameMacro("polygon model");

  // typedef referring to the parent resource.
  typedef smtk::model::Manager ParentResource;

  virtual ~Resource() {}

  void setSession(const Session::Ptr&);

  SessionPtr polygonSession();
  ConstSessionPtr polygonSession() const;

  void addStorage(
    const smtk::common::UUID& uid, smtk::bridge::polygon::internal::entity::Ptr storage);
  bool removeStorage(const smtk::common::UUID& uid);
  template <typename T>
  typename T::Ptr findStorage(const smtk::common::UUID& uid)
  {
    return this->polygonSession()->findStorage<T>(uid);
  }
  template <typename T>
  T findOrAddStorage(const smtk::common::UUID& uid)
  {
    return this->polygonSession()->findOrAddStorage<T>(uid);
  }
  int nextModelNumber() { return this->polygonSession()->m_nextModelNumber++; }

protected:
  Resource(const smtk::common::UUID&, smtk::resource::Manager::Ptr manager = nullptr);
  Resource(smtk::resource::Manager::Ptr manager = nullptr);

  Session::Ptr m_session;
};

} // namespace polygon
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_Resource_h
