//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_geometry_Manager_h
#define smtk_geometry_Manager_h

#include "smtk/geometry/Backend.h"
#include "smtk/geometry/Resource.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Observer.h"

#include "smtk/common/TypeName.h"
#include "smtk/common/UUID.h"

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"

#include <functional>
#include <string>
#include <typeinfo>
#include <unordered_map>

namespace smtk
{
namespace geometry
{
/**\brief Maintain a list of backends for which resources should provide geometry.
  *
  * A geometry manager accepts registration of different backends for
  * reprensenting geometry.
  * Given a resource manager, it observes resources added to the resource manager
  * and attempts to create geometry objects specific to registered backends for
  * each resource.
  *
  * Unlike other SMTK managers, this manager does not own or track instances
  * of Geometry objects; those objects have their lifetime tied to the life of
  * the resource which owns them.
  */
class SMTKCORE_EXPORT Manager : smtkEnableSharedPtr(Manager)
{
public:
  smtkTypedefs(smtk::geometry::Manager);
  smtkCreateMacro(Manager);

  virtual ~Manager() = default;

  /// Register a geometry backend type with the resource manager.
  template <typename Backend>
  bool registerBackend()
  {
    auto entry = std::make_shared<Backend>();
    if (m_backends.find(entry->index()) == m_backends.end())
    {
      m_backends[entry->index()] = entry;
      if (auto resourceManager = m_resourceManager.lock())
      {
        this->constructGeometry(resourceManager, *entry);
      }
      return true;
    }
    return false;
  }

  /// Unregister a geometry backend type with the resource manager.
  template <typename Backend>
  bool unregisterBackend()
  {
    Backend entry;
    auto it = m_backends.find(entry.index());
    if (it == m_backends.end())
    {
      return false;
    }
    m_backends.erase(it);
    return true;
  }

  /// Visit geometry backends that have been registered.
  void visitBackends(std::function<void(const Backend&)> visitor) const;

  /// Watch the given resource manager and add geometry objects to its resources as possible.
  void registerResourceManager(const smtk::resource::Manager::Ptr& manager);

protected:
  /// For any resources in \a resourceManager, attempt to construct geometry for \a backend.
  void constructGeometry(
    const std::shared_ptr<smtk::resource::Manager>& resourceManager, Backend& backend);

private:
  Manager() = default;

  /// The resource manager whose resources should have geometry added.
  std::weak_ptr<smtk::resource::Manager> m_resourceManager;
  /// The observer key used to add geometry as resources are added.
  smtk::resource::Observers::Key m_resourceObserverKey;
  /// A collection of backend flavors that geometry providers may come in.
  std::map<Backend::index_t, std::shared_ptr<Backend> > m_backends;
};

} // namespace geometry
} // namespace smtk

#endif // smtk_geometry_Manager_h
