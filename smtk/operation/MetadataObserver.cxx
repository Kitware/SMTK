//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/MetadataObserver.h"

#include "smtk/operation/Metadata.h"

#include <map>

namespace smtk
{
namespace operation
{

struct MetadataObservers::Internal
{
  std::map<MetadataObservers::Key, MetadataObserver> m_observers;
};

MetadataObservers::MetadataObservers()
  : m_internal(new Internal)
{
}

MetadataObservers::~MetadataObservers()
{
  delete m_internal;
}

void MetadataObservers::operator()(const Metadata& metadata)
{
  for (auto entry : m_internal->m_observers)
  {
    entry.second(metadata);
  }
}

MetadataObservers::Key MetadataObservers::insert(MetadataObserver fn)
{
  Key handle = m_internal->m_observers.empty() ? 0 : m_internal->m_observers.rbegin()->first + 1;
  return m_internal->m_observers.insert(std::make_pair(handle, fn)).second ? handle : -1;
}

std::size_t MetadataObservers::erase(MetadataObservers::Key handle)
{
  return m_internal->m_observers.erase(handle);
}

} // operation namespace
} // smtk namespace
