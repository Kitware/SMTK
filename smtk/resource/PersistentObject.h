//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_PersistentObject_h
#define smtk_resource_PersistentObject_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"

#include "smtk/common/UUID.h"

#include <string>

namespace smtk
{
namespace resource
{

/**\brief An abstract base class for SMTK resources and their components.
  *
  * Both resources and their components are intended to be accessed via
  * the resource system as black boxes using UUIDs. They are exposed
  * similarly in user interfaces (e.g., a tree view of SMTK state usually
  * includes both resources and their children), so it is useful to be
  * able to refer to either type interchangeably. Hence, this class exists
  * so that user interfaces can hold onto resources and components without
  * knowing or caring about their type.
  */
class SMTKCORE_EXPORT PersistentObject : smtkEnableSharedPtr(PersistentObject)
{
public:
  smtkTypeMacroBase(smtk::resource::PersistentObject);
  virtual ~PersistentObject();

  /// Return a unique identifier for the object which will be persistent across sessions.
  virtual const common::UUID& id() const = 0;
  /// Assign an ID to this object (used by readers; not for arbitrary reuse).
  virtual bool setId(const common::UUID& myID) = 0;
  /// Return the name of the object - by default it will return the UUID but that can be overriden
  virtual std::string name() const;

  /// Attempt to cast this object to a subclass.
  template <typename T>
  typename T::Ptr as()
  {
    return std::dynamic_pointer_cast<T>(shared_from_this());
  }
  /// Attempt to cast this object to a subclass.
  template <typename T>
  typename T::ConstPtr as() const
  {
    return std::dynamic_pointer_cast<const T>(shared_from_this());
  }

protected:
  PersistentObject();
};
}
}

#endif // smtk_resource_PersistentObject_h
