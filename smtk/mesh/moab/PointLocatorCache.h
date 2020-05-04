//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_mesh_moab_PointLocatorCache_h
#define smtk_mesh_moab_PointLocatorCache_h

#include "smtk/CoreExports.h"

#include "smtk/operation/queries/SynchronizedCache.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "moab/AdaptiveKDTree.hpp"
#include "moab/BoundBox.hpp"
#include "moab/CartVect.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

namespace smtk
{
namespace mesh
{
namespace moab
{

struct SMTKCORE_EXPORT PointLocatorCache : public smtk::operation::SynchronizedCache
{
  struct CacheForIndex
  {
    CacheForIndex(
      ::moab::Interface* interface, const ::moab::Range& range, ::moab::FileOptions* fileOptions)
      : m_interface(interface)
      , m_tree(m_interface, range, &m_treeRootSet, fileOptions)
    {
    }

    ::moab::Interface* m_interface;
    ::moab::EntityHandle m_treeRootSet;
    ::moab::AdaptiveKDTree m_tree;
  };

  PointLocatorCache() = default;
  ~PointLocatorCache() = default;
  PointLocatorCache(const PointLocatorCache&) = delete;
  PointLocatorCache(PointLocatorCache&& rhs)
    : m_caches(std::move(rhs.m_caches))
  {
  }

  PointLocatorCache& operator=(const PointLocatorCache&) = delete;
  PointLocatorCache& operator=(PointLocatorCache&& rhs)
  {
    m_caches = std::move(rhs.m_caches);
    return *this;
  }

  void synchronize(const smtk::operation::Operation&, const smtk::operation::Operation::Result&);

  std::unordered_map<smtk::common::UUID, std::unique_ptr<CacheForIndex> > m_caches;
};
}
}
}

#endif
