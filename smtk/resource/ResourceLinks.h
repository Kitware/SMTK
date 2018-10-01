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
class Resource;

namespace detail
{
class LinkBase : public Surrogate, public ComponentLinks::Data
{
public:
  LinkBase(const LinkBase&) = default;
  LinkBase(LinkBase&&) = default;

  LinkBase(const std::size_t& index, const std::string& typeName, const smtk::common::UUID& id,
    const std::string& location)
    : Surrogate(index, typeName, id, location)
    , ComponentLinks::Data()
  {
  }

  LinkBase(const ResourcePtr& resource)
    : Surrogate(resource)
    , ComponentLinks::Data()
  {
  }

  LinkBase(Surrogate&& surrogate)
    : Surrogate(surrogate)
    , ComponentLinks::Data()
  {
  }

  virtual ~LinkBase() {}
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
  typedef detail::LinkBase LinkBase;

public:
  friend class smtk::resource::Resource;

  typedef smtk::common::Links<smtk::common::UUID, smtk::common::UUID, smtk::common::UUID, int,
    LinkBase>
    ResourceLinkData;

  typedef ResourceLinkData::Link Link;

  ~ResourceLinks();

  /// Access the underlying link data.
  ResourceLinkData& data() { return m_data; }
  const ResourceLinkData& data() const { return m_data; }

private:
  ResourceLinks(Resource*);

  Resource* leftHandSideResource() override;
  const Resource* leftHandSideResource() const override;

  Resource* m_resource;
  ResourceLinkData m_data;
};
}
}
}

#endif // smtk_resource_ResourceLinks_h
