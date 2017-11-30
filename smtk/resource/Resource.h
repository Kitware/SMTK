//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_Resource_h
#define smtk_resource_Resource_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"
#include "smtk/common/UUID.h"

#include <string>
#include <typeindex>

namespace smtk
{
namespace resource
{
class Manager;
class Metadata;

/// An abstract base class for SMTK resources. Resources are one of: attribute
/// manager, model, mesh.
class SMTKCORE_EXPORT Resource : smtkEnableSharedPtr(Resource)
{
public:
  typedef std::size_t Index;
  typedef smtk::resource::Metadata Metadata;

  smtkTypeMacroBase(smtk::resource::Resource);
  virtual ~Resource();

  friend class Manager;

  virtual ComponentPtr find(const smtk::common::UUID& compId) const = 0;

  std::string uniqueName() const;

  // index is a compile-time intrinsic of the derived resource; as such, it
  // cannot be set.
  virtual Index index() const { return std::type_index(typeid(*this)).hash_code(); }

  // id and location are run-time intrinsics of the derived resource; we need to
  // allow the user to reset these values.
  const smtk::common::UUID& id() const { return this->m_id; }
  const std::string& location() const { return this->m_location; }

  bool setId(const smtk::common::UUID& myID);
  bool setLocation(const std::string& location);

  // Resources that are managed have a non-null pointer to their manager.
  ManagerPtr manager() const { return this->m_manager.lock(); }

protected:
  Resource(const smtk::common::UUID&, ManagerPtr manager = nullptr);
  Resource(ManagerPtr manager = nullptr);

private:
  smtk::common::UUID m_id;
  std::string m_location;

  WeakManagerPtr m_manager;
};
}
}

#endif // smtk_resource_Resource_h
