//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_bridge_polygon_internal_Entity_h
#define __smtk_bridge_polygon_internal_Entity_h

#include "smtk/SharedFromThis.h"
#include "smtk/bridge/polygon/PointerDefs.h"
#include "smtk/bridge/polygon/internal/Config.h"

namespace smtk
{
namespace bridge
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
class entity : smtkEnableSharedPtr(entity)
{
public:
  smtkTypeMacro(entity);

  Id id() const { return this->m_id; }
  void setId(const Id& i) { this->m_id = i; }

  entity* parent() const { return this->m_parent; }
  void setParent(entity* p) { this->m_parent = p; }

  template <typename T>
  T* parentAs() const
  {
    return dynamic_cast<T*>(this->m_parent);
  }

protected:
  entity()
    : m_parent(NULL)
  {
  }
  entity(const Id& uid, entity* p)
    : m_parent(p)
    , m_id(uid)
  {
  }
  virtual ~entity() { this->m_parent = NULL; }

  entity* m_parent;
  Id m_id;
};

} // namespace internal
} // namespace polygon
} // namespace bridge
} // namespace smtk

#endif // __smtk_bridge_polygon_internal_Entity_h
