//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#include "smtk/mesh/moab/HandleRangeToRange.h"

namespace smtk
{
namespace mesh
{
namespace moab
{

smtk::mesh::HandleRange moabToSMTKRange(const ::moab::Range& range)
{
  smtk::mesh::HandleRange handleRange;
  for (::moab::Range::pair_iterator it = range.begin(); it != range.end(); ++it)
  {
    handleRange.add(handleRange.end(), HandleInterval(it->first, it->second));
  }
  return handleRange;
}

::moab::Range smtkToMOABRange(const smtk::mesh::HandleRange& handleRange)
{
  ::moab::Range range;

  // As per the advice in Moab's Range.hpp documentation, we insert ranges from
  // largest to smallest.
  for (auto it = handleRange.rbegin(); it != handleRange.rend(); ++it)
  {
    range.insert(range.begin(), it->lower(), it->upper());
  }
  return range;
}
}
}
}
