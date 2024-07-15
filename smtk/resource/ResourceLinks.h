//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_ResourceLinks_h
#define smtk_resource_ResourceLinks_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/Links.h"
#include "smtk/common/UUID.h"

#include "smtk/resource/ComponentLinks.h"
#include "smtk/resource/Surrogate.h"

#include "smtk/resource/Links.h"

#include <set>

namespace smtk
{
namespace resource
{
class CopyOptions;
class Resource;

namespace detail
{
class ResourceLinkBase
  : public Surrogate
  , public ComponentLinks::Data
{
public:
  ResourceLinkBase(const ResourceLinkBase&) = default;
  ResourceLinkBase(ResourceLinkBase&&) = default;

  ResourceLinkBase(
    const std::size_t& index,
    const std::string& typeName,
    const smtk::common::UUID& id,
    const std::string& location)
    : Surrogate(index, typeName, id, location)
  {
  }

  ResourceLinkBase(const ResourcePtr& resource)
    : Surrogate(resource)
  {
  }

  ResourceLinkBase(Surrogate&& surrogate)
    : Surrogate(surrogate)
  {
  }

  ~ResourceLinkBase() override = default;
};

/// The ResourceLinks class is a resource-specific API for manipulating
/// unidirectional links from a Resource and its Components to other Resources
/// and Components. Internally, the storage structure for links is an
/// smtk::common::Links instance describing links between resources, and each
/// link in this structure is an smtk::common::Links instance describing links
/// between components.
class SMTKCORE_EXPORT ResourceLinks : public Links
{
  typedef detail::ComponentLinks::Data ComponentLinkData;
  typedef detail::ResourceLinkBase ResourceLinkBase;

public:
  friend class smtk::resource::Resource;

  typedef smtk::common::
    Links<smtk::common::UUID, smtk::common::UUID, smtk::common::UUID, int, ResourceLinkBase>
      ResourceLinkData;

  typedef ResourceLinkData::Link Link;

  ~ResourceLinks() override;

  /// Access the underlying link data.
  ResourceLinkData& data() { return m_data; }
  const ResourceLinkData& data() const { return m_data; }

  /// Resolve any surrogates with the given resource. Returns true if a surrogate
  /// is successfully resolved.
  bool resolve(const ResourcePtr&) const;

  /// Remove all links from this resource to another resource. This is useful
  /// when we know the resource parameter is being permanently deleted.
  bool removeAllLinksTo(const ResourcePtr&);

  /// Copy links from \a source's links with the given \a options.
  ///
  /// This will copy both resource and component links as \a options indicates.
  /// Both resources and components are mapped through the \a options' object-mapping,
  /// so any links to objects that are also being copied will point to the copy
  /// and not the original object in \a source.
  void copyFrom(const ConstResourcePtr& source, const smtk::resource::CopyOptions& options);

private:
  ResourceLinks(Resource*);

  Resource* leftHandSideResource() override;
  const Resource* leftHandSideResource() const override;

  Resource* m_resource;
  ResourceLinkData m_data;
};
} // namespace detail
} // namespace resource
} // namespace smtk

#endif // smtk_resource_ResourceLinks_h
