//=========================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.

// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_geometry_Query_h
#define smtk_geometry_Query_h

#include "smtk/CoreExports.h"

#include "smtk/common/UUID.h"

#include <limits>

namespace smtk
{
namespace resource
{
namespace query
{

class Metadata;

/// A base class for Query functors.
class SMTKCORE_EXPORT Query
{
  friend class Metadata;

public:
  static std::size_t typeIndex();
  virtual ~Query() = default;

protected:
  static int numberOfGenerationsFromType(std::size_t index);
};

} // namespace query
} // namespace resource
} // namespace smtk

#endif
