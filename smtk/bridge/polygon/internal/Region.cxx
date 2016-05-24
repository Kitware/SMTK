//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/polygon/internal/Region.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

Region::Region()
  : m_seedFragment(-1), m_seedSense(true)
{
}

Region::Region(FragmentId seedFrag, bool seedSense)
  : m_seedFragment(seedFrag), m_seedSense(seedSense)
{
}

void Region::merge(const Region* other)
{
  if (!other)
    {
    return;
    }
  if (this->m_seedFragment == -1)
    {
    this->m_seedFragment = other->m_seedFragment;
    this->m_seedSense = other->m_seedSense;
    for (std::set<int>::const_iterator it = other->m_innerLoops.begin(); it != other->m_innerLoops.end(); ++it)
      {
      this->m_innerLoops.insert(*it);
      }
    }
}

    } // namespace polygon
  } //namespace bridge
} // namespace smtk
