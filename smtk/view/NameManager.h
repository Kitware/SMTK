//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_NameManager_h
#define smtk_view_NameManager_h

#include "smtk/CoreExports.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"

#include <memory>
#include <string>

namespace smtk
{
namespace resource
{
class PersistentObject;
}
namespace view
{

/**\brief An application-wide utility for assigning unique names to objects.
  *
  * This is a simple counter that names objects based on their type name plus
  * a unique integer that is auto-incremented each time a name is assigned.
  * Thus no two objects, regardless of their type, will be assigned the same
  * integer as long as a session is in memory or the counter is serialized
  * with the resource.
  *
  * Your application is expected to create an instance of a NameManager
  * and add it to an smtk::common::Managers instance for resources to use.
  * Some of the markup resource's operations will use a NameManager if it
  * is present in the Managers object passed to them.
  * A good way to automatically add a NameManager is inside the
  * `Registrar::registerTo(const smtk::common::Managers::Ptr&)` method
  * for some registrar in a plugin unique to your application.
  * This is **not** done inside the `smtk::view::Registrar` as that
  * would force the NameManager to be present for all applications.
  */
class SMTKCORE_EXPORT NameManager : smtkEnableSharedPtr(NameManager)
{
public:
  virtual ~NameManager() = default;
  smtkTypeMacroBase(smtk::view::NameManager);
  smtkCreateMacro(smtk::view::NameManager);

  std::string nameForObject(const smtk::resource::PersistentObject& obj);

  /// Return the current counter without incrementing it.
  /// This is intended solely for serializing the manager to disk.
  int counter() const { return m_counter; }

  /// Set the counter used to name objects.
  /// This is intended solely for deserializing the manager from disk.
  void resetCounter(int value) { m_counter = value; }

protected:
  NameManager();

  int m_counter{ 1 };
};

} // namespace view
} // namespace smtk
#endif
