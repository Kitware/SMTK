//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_Links_h
#define smtk_resource_Links_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

namespace smtk
{
namespace resource
{
class Component;
class Resource;

/// Links is a pure virtual class describing the API for connecting one
/// resource/component to another resource/component.
class SMTKCORE_EXPORT Links
{
public:
  /// A Key is a pair of UUIDs. the First UUID is the id of the resource link,
  /// and the second one is the id of the component link.
  typedef std::pair<smtk::common::UUID, smtk::common::UUID> Key;
  typedef int RoleType;

  /// Given a resource or component and a role, check if a link of this role type
  /// exists between us and the input object.
  bool isLinkedTo(const ResourcePtr&, const RoleType&) const;
  bool isLinkedTo(const ComponentPtr&, const RoleType&) const;

  /// Given a resource or component and a role type, construct a link from
  /// us to the input object and assign the link a random UUID. Return a key
  /// that uniquely identifies the link if successfull, or return a key
  /// comprised of a pair of null UUIDs if the link construction failed.
  Key addLinkTo(const ResourcePtr&, const RoleType&);
  Key addLinkTo(const ComponentPtr&, const RoleType&);

  /// Given a role, return a set of objects to which this object links.
  PersistentObjectSet linkedTo(const RoleType&) const;

  /// Given a role and a resource corresponding to the rhs of a link, return a
  /// set of objects from the rhs that link to this object using this role type.
  PersistentObjectSet linkedFrom(const ResourcePtr&, const RoleType&) const;

  /// Given a role, return a set of objects that link to this object using this
  /// role type. Because links are unidirectional, this method queries every
  /// resource managed by the accessible manager (the one managing lhs1) and
  /// queries their links for links to this object. If the resource associated
  /// with this link object is not managed, this method returns an empty set.
  PersistentObjectSet linkedFrom(const RoleType&) const;

  /// Given a Link key, remove the associated link. Return true if successful.
  bool removeLink(const Key&);

  /// Given a Link key, return the object and role to which this object is
  /// linked, or return nullptr if no link exists with this link id.
  std::pair<PersistentObjectPtr, RoleType> linkedObjectAndRole(const Key&) const;
  PersistentObjectPtr linkedObject(const Key& key) const { return linkedObjectAndRole(key).first; }

protected:
  virtual Resource* leftHandSideResource() = 0;
  virtual const Resource* leftHandSideResource() const = 0;

  virtual const smtk::common::UUID& leftHandSideComponentId() const;

private:
  bool isLinkedTo(const Resource* lhs1, const smtk::common::UUID& lhs2,
    const smtk::common::UUID& rhs1, const smtk::common::UUID& rhs2, const RoleType& role) const;

  Key addLinkTo(Resource* lhs1, const smtk::common::UUID& lhs2, const ResourcePtr& rhs1,
    const smtk::common::UUID& rhs2, const RoleType& role);

  PersistentObjectSet linkedTo(
    const Resource* lhs1, const smtk::common::UUID& lhs2, const RoleType& role) const;

  PersistentObjectSet linkedFrom(const ResourcePtr& lhs1, const Resource* rhs1,
    const smtk::common::UUID& rhs2, const RoleType& role) const;

  PersistentObjectSet linkedFrom(
    const Resource* rhs1, const smtk::common::UUID& rhs2, const RoleType& role) const;

  bool removeLink(Resource* lhs1, const Key& key);

  std::pair<PersistentObjectPtr, Links::RoleType> linkedObjectAndRole(
    const Resource*, const Key&) const;
};
}
}

#endif // smtk_resource_Links_h
