//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_session_polygon_internal_Entity_h
#define __smtk_session_polygon_internal_Entity_h

#include "smtk/SharedFromThis.h"
#include "smtk/session/polygon/Exports.h"
#include "smtk/session/polygon/PointerDefs.h"
#include "smtk/session/polygon/internal/Config.h"

namespace smtk
{
namespace session
{
namespace polygon
{
namespace internal
{

/**\brief A base class for all internal entity storage.
  *
  * Every entity stores a pointer to its parent and its UUID.
  * This class uses smtkEnableSharedPtr so that all entities may be
  * managed via one pool of shared pointers.
  */
class SMTKPOLYGONSESSION_EXPORT entity : smtkEnableSharedPtr(entity)
{
public:
  smtkTypeMacroBase(entity);

  Id id() const { return m_id; }
  void setId(const Id& i) { m_id = i; }

  entity* parent() const { return m_parent; }
  void setParent(entity* p) { m_parent = p; }

  template<typename T>
  T* parentAs() const
  {
    return dynamic_cast<T*>(m_parent);
  }

protected:
  entity()
    : m_parent(nullptr)
  {
  }
  entity(const Id& uid, entity* p)
    : m_parent(p)
    , m_id(uid)
  {
  }
  virtual ~entity() { m_parent = nullptr; }

  entity* m_parent;
  Id m_id;
};

} // namespace internal
} // namespace polygon
} // namespace session
} // namespace smtk

#endif // __smtk_session_polygon_internal_Entity_h
