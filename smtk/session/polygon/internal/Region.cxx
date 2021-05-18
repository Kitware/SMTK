//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/polygon/internal/Region.h"

namespace smtk
{
namespace session
{
namespace polygon
{

Region::Region()
  : m_seedFragment(static_cast<FragmentId>(-1))
  , m_seedSense(true)
{
}

Region::Region(FragmentId seedFrag, bool seedSense)
  : m_seedFragment(seedFrag)
  , m_seedSense(seedSense)
{
}

void Region::merge(const Region* other)
{
  if (!other)
  {
    return;
  }
  if (m_seedFragment == static_cast<FragmentId>(-1))
  {
    m_seedFragment = other->m_seedFragment;
    m_seedSense = other->m_seedSense;
    for (std::set<int>::const_iterator it = other->m_innerLoops.begin();
         it != other->m_innerLoops.end();
         ++it)
    {
      m_innerLoops.insert(*it);
    }
  }
}

} // namespace polygon
} //namespace session
} // namespace smtk
