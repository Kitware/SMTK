//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_ComponentLInks_h
#define smtk_resource_ComponentLInks_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/Links.h"
#include "smtk/common/UUID.h"

namespace smtk
{
namespace resource
{
class Component;
class Resource;

/// The ComponentLinks class is a component-specific API for maninpulating
/// unidirectional links from a component to other Resources and Components.
/// Internally, the class is stateless; all calls are performed through the
/// component's resource.
class SMTKCORE_EXPORT ComponentLinks
{
public:
  friend class Component;

  typedef smtk::common::Links<smtk::common::UUID> Data;

  /// A Key is a pair of UUIDs. the First UUID is the id of the resource link,
  /// and the second one is the id of the component link.
  typedef std::pair<smtk::common::UUID, smtk::common::UUID> Key;

  /// Given a resource or component, check if a link exists between this
  /// component and the input parameter.
  bool isLinkedTo(const ResourcePtr&) const;
  bool isLinkedTo(const ComponentPtr&) const;

  /// Given a resource or component, construct a resource component link from
  /// this component to the input and assign the link a random UUID. Return a
  /// key composed of the resource link id and component link id (when linking
  /// to a resource, the component id is set to a reserved value), or return a
  /// key comprised of a pair of null UUIDs if the link construction failed.
  Key addLinkTo(const ResourcePtr&, const std::string&);
  Key addLinkTo(const ComponentPtr&, const std::string&);

  /// Given a Link key, remove the associated link. Return true if successful.
  bool removeLink(const Key&);

  /// Given a Link key, return the resource and role to which this component is
  /// linked, or return nullptr if (a) no link exists with this
  /// link id, (b) the link does not have this component as the "from"
  /// component, or (c) the link is not a resource link (i.e. it is a component
  /// link).
  std::pair<ResourcePtr, std::reference_wrapper<const std::string> > linkedResource(
    const Key&) const;

  /// Given a Link key, return the resource component and role to which this
  /// component is linked, or return nullptr if (a) no link exists with this
  /// link id, (b) the link does not have this component as the "from"
  /// component or (c) the link is not a component link (i.e. it is a resource
  /// link).
  std::pair<ComponentPtr, std::reference_wrapper<const std::string> > linkedComponent(
    const Key&) const;

private:
  ComponentLinks(const Component*);

  /// Given a resource id and a component id, check if a link exists between
  /// this component and the input resource component.
  bool isLinkedTo(const smtk::common::UUID&, const smtk::common::UUID&) const;

  /// Given a resource and a component id, construct a resource component link
  /// from this component to the input and assign the link a random UUID. Return
  /// a key composed of the resource link id and component link id (when linking
  /// to a resource, the component id is set to a reserved value).
  Key addLinkTo(const ResourcePtr&, const smtk::common::UUID&, const std::string&);

  const Component* m_component;
};
}
}

#endif // smtk_resource_ComponentLinks_h
