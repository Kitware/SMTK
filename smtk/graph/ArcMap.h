//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_ArcMap_h
#define smtk_graph_ArcMap_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/TypeMap.h"

namespace smtk
{
namespace graph
{

/**\brief A container for arcs held by a resource.
  *
  * The main reason this currently exists is to delete the copy/assignment
  * constructors so developers must reference the container instead of
  * mistakenly modifying an accidental copy.
  */
class SMTKCORE_EXPORT ArcMap : public smtk::common::TypeMap<smtk::common::UUID>
{
public:
  smtkTypeMacroBase(smtk::graph::ArcMap);
  smtkSuperclassMacro(smtk::common::TypeMap<smtk::common::UUID>);
  using key_type = smtk::common::UUID;

  ArcMap() = default;

  using smtk::common::TypeMap<smtk::common::UUID>::TypeMap;

  /// Do not allow the map to be copied:
  ArcMap(const ArcMap&) = delete;
  ArcMap& operator=(const ArcMap&) = delete;

  ~ArcMap() override = default;
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_ArcMap_h
