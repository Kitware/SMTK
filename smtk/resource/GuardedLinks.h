//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_GuardedLinks_h
#define smtk_resource_GuardedLinks_h

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

#include <mutex>

namespace smtk
{
namespace resource
{

/// Thread-safe access to links.
template<typename LinkType>
class GuardedLinks
{
public:
  GuardedLinks(std::mutex& mutex, const LinkType& links)
    : m_guard(mutex)
    , m_links(links)
  {
  }

  const LinkType* operator->() const { return &m_links; }

  LinkType* operator->() { return const_cast<LinkType*>(&m_links); }

private:
  std::unique_lock<std::mutex> m_guard;
  const LinkType& m_links;
};

/// Type-aliases for guarding specific types of links (resource vs. component).
using GuardedResourceLinks = GuardedLinks<smtk::resource::Resource::Links>;
using GuardedComponentLinks = GuardedLinks<smtk::resource::Component::Links>;

} // namespace resource
} // namespace smtk

#endif // smtk_resource_GuardedLinks_h
