//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/resource/ResourceLinks.h"

#include "smtk/resource/ComponentLinks.h"
#include "smtk/resource/CopyOptions.h"
#include "smtk/resource/Manager.h"

#include "smtk/common/UUID.h"

#include "smtk/resource/Resource.h"

#include <algorithm>

// Uncomment to get debug printouts
// #define SMTK_DBG_COPYLINKS

namespace smtk
{
namespace resource
{
namespace detail
{
ResourceLinks::ResourceLinks(Resource* resource)
  : m_resource(resource)
{
  // NOTE: When modifying this constructor, do not use the resource parameter
  // or m_resource field! The parent Resouce is still in construction and is in
  // an indeterminate state.
}

ResourceLinks::~ResourceLinks() = default;

Resource* ResourceLinks::leftHandSideResource()
{
  return m_resource;
}

const Resource* ResourceLinks::leftHandSideResource() const
{
  return m_resource;
}

bool ResourceLinks::resolve(const ResourcePtr& resource) const
{
  auto i = std::find_if(m_data.begin(), m_data.end(), [&resource](const Surrogate& surrogate) {
    return surrogate.typeName() == resource->typeName() && surrogate.id() == resource->id();
  });
  if (i == m_data.end())
  {
    return false;
  }
  i->resolve(resource);
  return true;
}

bool ResourceLinks::removeAllLinksTo(const ResourcePtr& resource)
{
  return this->data().erase_all<ResourceLinkData::Right>(resource->id());
}

void ResourceLinks::copyFrom(
  const ConstResourcePtr& source,
  const smtk::resource::CopyOptions& options)
{
  if (!options.copyLinks() || !source || !m_resource)
  {
    // Skip copying link data if told to.
    return;
  }

  smtk::resource::Manager::Ptr rsrcMgr = m_resource->manager();
  if (!rsrcMgr)
  {
    rsrcMgr = source->manager();
  }

  // First, copy top-level, resource-to-resource connections.
  // These are always copied, even if no component/resource links
  // underneath them are copied. We may add an option later to
  // avoid this.
  const auto& srcData = source->links().data();
  for (const auto& entry : srcData)
  {
    auto targetRightResourceId = entry.right;
    if (options.shouldOmitId(targetRightResourceId))
    {
      smtkInfoMacro(options.log(), "Skipping omitted resource " << targetRightResourceId << ".");
      continue;
    }
    Resource::Ptr targetRightResource;
    if (targetRightResourceId == source->id())
    {
      targetRightResourceId = m_resource->id();
      targetRightResource = const_pointer_cast<Resource>(m_resource->shared_from_this());
    }
    else if (auto* mapped = options.targetObjectFromSourceId<Resource>(targetRightResourceId))
    {
      targetRightResourceId = mapped->id();
      targetRightResource = mapped->shared_from_this();
    }
    else
    {
      // Look up resource in resource manager or omit the
      // copied links; we need the resource to be loaded
      // to copy the links properly since they include pointers.
      if (rsrcMgr)
      {
        targetRightResource = rsrcMgr->get(targetRightResourceId);
      }
      // TODO: Handle Surrogates by fetching? Add a copy-option to avoid fetching?
      if (!targetRightResource)
      {
        smtkWarningMacro(options.log(), "No resource found for " << targetRightResourceId << ".");
        continue;
      }
    }
    // Iterate over children of top-level link.
#ifdef SMTK_DBG_COPYLINKS
    std::cerr << "Translate " << entry.left << " → " << entry.right << " (" << entry.role << ")\n"
              << "     into " << m_resource->id() << " → " << targetRightResourceId << "\n";
#endif
    for (const auto& subentry : entry.get<ResourceLinkData::Right>())
    {
      // Check whether to copy the sublink.
      if (
        options.shouldExcludeLinksInRole(subentry.role) || options.shouldOmitId(subentry.left) ||
        options.shouldOmitId(subentry.right) || subentry.role == Links::invalidRole())
      {
        smtkInfoMacro(
          options.log(),
          "Skipping link (" << subentry.left << " → " << subentry.right << " " << subentry.role
                            << ") due to component in 'omit' list "
                            << "or excluded/invalid role.");
        continue;
      }
      auto idLf = subentry.left;
      auto idRt = subentry.right;
      auto* mappedLf = options.targetObjectFromSourceId<PersistentObject>(idLf);
      auto* mappedRt = options.targetObjectFromSourceId<PersistentObject>(idRt);
      if (mappedLf)
      {
        idLf = mappedLf->id();
      }
      else if (!mappedLf && subentry.left)
      {
        // We should never copy a link whose left ("from") entry is not present in the
        // target resource. Verify that idLf exists as a component in our resource.
        if (!m_resource->component(idLf))
        {
          smtkWarningMacro(
            options.log(),
            "Left-hand side UUID " << subentry.left
                                   << " has no mapping and "
                                      "is not in target resource. Skipping.");
          continue;
        }
      }
      if (mappedRt)
      {
        idRt = mappedRt->id();
      }
#ifdef SMTK_DBG_COPYLINKS
      std::cerr << "     Sublink " << subentry.left << " → " << subentry.right << " ("
                << subentry.role << ")\n"
                << "          to " << idLf << " → " << idRt << " (" << subentry.role << ")\n";
#endif
      this->addLinkTo(m_resource, idLf, targetRightResource, idRt, subentry.role);
    }
  }
}

} // namespace detail
} // namespace resource
} // namespace smtk
