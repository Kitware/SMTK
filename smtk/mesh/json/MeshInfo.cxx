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
#include "smtk/mesh/json/MeshInfo.h"

#include "smtk/mesh/moab/CellTypeToType.h"

namespace smtk
{
namespace mesh
{
namespace json
{

MeshInfo::MeshInfo()
  : m_mesh()
  , m_uuid()
  , m_cells()
  , m_points()
  , m_types()
  , m_domains()
  , m_dirichlets()
  , m_neumanns()
  , m_uuids()
{
}

MeshInfo::MeshInfo(smtk::mesh::Handle meshId, const smtk::common::UUID& uuid,
  const smtk::mesh::HandleRange& cells, const smtk::mesh::HandleRange& points,
  smtk::mesh::TypeSet types)
  : m_mesh(meshId)
  , m_uuid(uuid)
  , m_cells(cells)
  , m_points(points)
  , m_types(types)
  , m_domains()
  , m_dirichlets()
  , m_neumanns()
  , m_uuids()
{
}

smtk::mesh::HandleRange MeshInfo::cells() const
{
  return m_cells;
}

smtk::mesh::HandleRange MeshInfo::cells(smtk::mesh::CellType cellType) const
{
  const int moabCellType = smtk::mesh::moab::smtkToMOABCell(cellType);
  return m_cells.subset_by_type(static_cast< ::moab::EntityType>(moabCellType));
}

smtk::mesh::HandleRange MeshInfo::cells(const smtk::mesh::CellTypes& cellTypes) const
{
  smtk::mesh::HandleRange entitiesCells;
  for (int i = static_cast<int>(cellTypes.size() - 1); i >= 0; --i)
  {
    //skip all cell types we don't have
    if (!cellTypes[i])
    {
      continue;
    }

    smtk::mesh::CellType currentCellType = static_cast<smtk::mesh::CellType>(i);

    smtk::mesh::HandleRange cellEnts = this->cells(currentCellType);
    entitiesCells.insert(cellEnts.begin(), cellEnts.end());
  }
  return entitiesCells;
}

smtk::mesh::HandleRange MeshInfo::cells(smtk::mesh::DimensionType dim) const
{
  return m_cells.subset_by_dimension(static_cast<int>(dim));
}

smtk::mesh::HandleRange MeshInfo::points() const
{
  return m_points;
}

bool MeshInfo::has(const smtk::mesh::Domain& d) const
{
  return std::find(m_domains.begin(), m_domains.end(), d) != m_domains.end();
}

bool MeshInfo::has(const smtk::mesh::Dirichlet& bc) const
{
  return std::find(m_dirichlets.begin(), m_dirichlets.end(), bc) != m_dirichlets.end();
}

bool MeshInfo::has(const smtk::mesh::Neumann& bc) const
{
  return std::find(m_neumanns.begin(), m_neumanns.end(), bc) != m_neumanns.end();
}
}
}
}
