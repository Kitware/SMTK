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
#ifndef smtk_mesh_json_MeshInfo_h
#define smtk_mesh_json_MeshInfo_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/core/CellTypes.h"
#include "smtk/mesh/core/DimensionTypes.h"
#include "smtk/mesh/core/Handle.h"
#include "smtk/mesh/core/TypeSet.h"

namespace smtk
{
namespace mesh
{
namespace json
{

class SMTKCORE_EXPORT MeshInfo
{
public:
  MeshInfo();

  MeshInfo(
    smtk::mesh::Handle meshId,
    const smtk::common::UUID& uuid,
    const smtk::mesh::HandleRange& cells,
    const smtk::mesh::HandleRange& points,
    smtk::mesh::TypeSet types);

  smtk::mesh::Handle mesh() const { return m_mesh; }

  smtk::mesh::TypeSet types() const { return m_types; }

  smtk::mesh::HandleRange cells() const;
  smtk::mesh::HandleRange cells(smtk::mesh::CellType cellType) const;
  smtk::mesh::HandleRange cells(const smtk::mesh::CellTypes& cellTypes) const;
  smtk::mesh::HandleRange cells(smtk::mesh::DimensionType dim) const;

  smtk::mesh::HandleRange points() const;

  bool has(const smtk::mesh::Domain& d) const;
  bool has(const smtk::mesh::Dirichlet& bc) const;
  bool has(const smtk::mesh::Neumann& bc) const;

  const std::vector<smtk::mesh::Domain>& domains() const { return m_domains; }
  const std::vector<smtk::mesh::Dirichlet>& dirichlets() const { return m_dirichlets; }
  const std::vector<smtk::mesh::Neumann>& neumanns() const { return m_neumanns; }
  const smtk::common::UUIDArray& modelUUIDS() const { return m_uuids; }
  const smtk::common::UUID& id() const { return m_uuid; }

  void set(const std::vector<smtk::mesh::Domain>& ds) { m_domains = ds; }
  void set(const std::vector<smtk::mesh::Dirichlet>& bcs) { m_dirichlets = bcs; }
  void set(const std::vector<smtk::mesh::Neumann>& bcs) { m_neumanns = bcs; }
  void set(const smtk::common::UUIDArray& array) { m_uuids = array; }

  bool operator==(const smtk::mesh::Handle& other) const { return other == m_mesh; }

private:
  smtk::mesh::Handle m_mesh;
  smtk::common::UUID m_uuid;
  smtk::mesh::HandleRange m_cells;
  smtk::mesh::HandleRange m_points;
  smtk::mesh::TypeSet m_types;

  std::vector<smtk::mesh::Domain> m_domains;
  std::vector<smtk::mesh::Dirichlet> m_dirichlets;
  std::vector<smtk::mesh::Neumann> m_neumanns;
  smtk::common::UUIDArray m_uuids;
};
} // namespace json
} // namespace mesh
} // namespace smtk

#endif
