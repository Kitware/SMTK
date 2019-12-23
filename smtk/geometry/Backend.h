//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_geometry_Backend_h
#define smtk_geometry_Backend_h

#include "smtk/CoreExports.h"

#include <string>
#include <typeindex>

namespace smtk
{
namespace geometry
{

/**\brief This is an empty class used by geometry providers to specify their interface.
  *
  * \sa smtk::extension::vtk::source::Backend
  */
class SMTKCORE_EXPORT Backend
{
public:
  using index_t = std::size_t;
  virtual ~Backend() {}

  /// Index is a compile-time intrinsic of the derived backend. It is used for disambiguation of backends.
  virtual index_t index() const { return std::type_index(typeid(*this)).hash_code(); }

  /// As a convenience (for serialization and presentation), each backend should provide a unique name.
  virtual std::string name() const = 0;
};

} // namespace resource
} // namespace smtk

#endif // smtk_geometry_Backend_h
