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
#include "smtk/resource/Links.h"
#include "smtk/resource/Properties.h"

#include <string>

namespace smtk
{
namespace resource
{

class Resource;

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
  /// TODO: care must be taken to modify the recorded ID in all links that
  ///       connect to this object (see Resource::setId and its treatment of
  ///       manager registration for reference).
  virtual bool setId(const common::UUID& myID) = 0;
  /// Return the name of the object - by default it will return the UUID but that can be overridden
  virtual std::string name() const;

  /**\brief Return a raw (not shared) pointer to the resource that owns this object.
    *
    *
    * Note that not all objects will have an owning resource.
    */
  virtual Resource* parentResource() const = 0;

  /// Attempt to cast this object to a subclass.
  template<typename T>
  typename T::Ptr as()
  {
    return std::dynamic_pointer_cast<T>(shared_from_this());
  }
  /// Attempt to cast this object to a subclass.
  template<typename T>
  typename T::ConstPtr as() const
  {
    return std::dynamic_pointer_cast<const T>(shared_from_this());
  }

  virtual smtk::resource::Links& links() = 0;
  virtual const smtk::resource::Links& links() const = 0;

  virtual smtk::resource::Properties& properties() = 0;
  virtual const smtk::resource::Properties& properties() const = 0;

protected:
  PersistentObject();
};
} // namespace resource
} // namespace smtk

#endif // smtk_resource_PersistentObject_h
