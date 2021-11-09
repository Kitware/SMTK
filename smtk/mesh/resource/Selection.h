//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_mesh_Selection_h
#define smtk_mesh_Selection_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/TupleTraits.h"

#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/Component.h"

namespace smtk
{
namespace mesh
{

/// A lightweight object for representing meshset information as a transient
/// resource component.
class SMTKCORE_EXPORT Selection : public smtk::mesh::Component
{
public:
  smtkTypeMacro(Selection);
  smtkSharedFromThisMacro(smtk::resource::Component);

  Selection(const smtk::mesh::CellSet&);
  ~Selection() override;

  static std::shared_ptr<Selection> create(const smtk::mesh::CellSet&);
  static std::shared_ptr<Selection> create(
    const smtk::mesh::CellSet&,
    const std::weak_ptr<smtk::operation::Manager>&);

  /// Access the meshset represented by this component.
  const smtk::mesh::MeshSet mesh() const override;
  smtk::mesh::MeshSet mesh() override;

  std::string name() const override;

private:
  // Modify the access level of links to prevent their explicit use.
  Links& links() override { return smtk::resource::Component::links(); }
  const Links& links() const override { return smtk::resource::Component::links(); }

  smtk::mesh::HandleRange m_cells;
};

} // namespace mesh
} // namespace smtk

#endif
