//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_Component_h
#define smtk_resource_Component_h

#include "smtk/resource/ComponentLinks.h"
#include "smtk/resource/PersistentObject.h"
#include "smtk/resource/Properties.h"

#include <string>

namespace smtk
{
namespace resource
{
class Resource;

/**\brief Component is the base class for records stored in an smtk::resource::Resource.
  *
  * Components are contained inside a parent resource.
  * Because they are written to and read from the resource's location,
  * their scope for persistence is shared with the resource;
  * SMTK does not provide a way to persist individual components,
  * only resources.
  *
  * A component in one resource may be linked to an external resource
  * or to components in an external resource using Links.
  * This allows one resource to annotate another.
  *
  * Components may themselves be annotated with Properties,
  * which are freeform storage owned by their parent resource.
  *
  * Attributes, mesh sets, and model entities all inherit from this class.
  */
class SMTKCORE_EXPORT Component : public PersistentObject
{
  friend class Resource;

public:
  typedef std::function<void(const ComponentPtr&)> Visitor;
  typedef detail::ComponentLinks Links;
  typedef detail::ComponentProperties Properties;

  smtkTypeMacro(smtk::resource::Component);
  smtkSuperclassMacro(smtk::resource::PersistentObject);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  ~Component() override;

  /**\brief Components are required to return their owning resource (if any).
    *
    * Components may not have an owning resource at some points in their
    * lifecycle, so be sure to verify the returned value is non-null.
    */
  virtual const ResourcePtr resource() const = 0;

  /**\brief Return a raw (not shared) pointer to the resource that owns this component.
    *
    * This method provides a slow default implementation that simply
    * converts the shared pointer, but component subclasses can usually
    * implement a fast version if they use smtk::common::CachedWeakPointer
    * to hold a reference to their parent.
    */
  virtual Resource* parentResource() const { return this->resource().get(); }

  /// Return the links that connect this component to external resources/components.
  Links& links() override { return m_links; }
  const Links& links() const override { return m_links; }

  /// Return properties defined on this component.
  Properties& properties() override { return m_properties; }
  const Properties& properties() const override { return m_properties; }

protected:
  Component();

private:
  Links m_links;
  Properties m_properties;
};
} // namespace resource
} // namespace smtk

#endif // smtk_resource_Component_h
