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
#ifndef __smtk_mesh_json_MeshInfo_h
#define __smtk_mesh_json_MeshInfo_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/CellTypes.h"
#include "smtk/mesh/DimensionTypes.h"
#include "smtk/mesh/Handle.h"
#include "smtk/mesh/TypeSet.h"


namespace smtk {
namespace mesh {
namespace json {

class SMTKCORE_EXPORT MeshInfo
{
public:
  MeshInfo();

  MeshInfo( smtk::mesh::Handle meshId,
            const smtk::mesh::HandleRange& cells,
            const smtk::mesh::HandleRange& points,
            smtk::mesh::TypeSet types);

  smtk::mesh::Handle mesh() const { return this->m_mesh; }

  smtk::mesh::TypeSet types() const { return this->m_types; }

  smtk::mesh::HandleRange cells() const;
  smtk::mesh::HandleRange cells(smtk::mesh::CellType cellType) const;
  smtk::mesh::HandleRange cells(const smtk::mesh::CellTypes& cellTypes) const;
  smtk::mesh::HandleRange cells(smtk::mesh::DimensionType dim) const;

  bool has(const smtk::mesh::Domain &d) const;
  bool has(const smtk::mesh::Dirichlet &bc) const;
  bool has(const smtk::mesh::Neumann &bc) const;

  const std::vector<smtk::mesh::Domain>& domains() const { return this->m_domains; }
  const std::vector<smtk::mesh::Dirichlet>& dirichlets() const { return this->m_dirichlets; }
  const std::vector<smtk::mesh::Neumann>& neumanns() const { return this->m_neumanns; }
  const smtk::common::UUIDArray& modelUUIDS() const { return this->m_uuids; }

  void set(const std::vector<smtk::mesh::Domain> &ds)       { this->m_domains = ds; }
  void set(const std::vector<smtk::mesh::Dirichlet> &bcs)   { this->m_dirichlets = bcs; }
  void set(const std::vector<smtk::mesh::Neumann> &bcs)     { this->m_neumanns = bcs; }
  void set(const smtk::common::UUIDArray& array)            { this->m_uuids = array; }

  bool operator==(const smtk::mesh::Handle& other) const
    { return other == this->m_mesh; }

private:
  smtk::mesh::Handle m_mesh;
  smtk::mesh::HandleRange m_cells;
  smtk::mesh::HandleRange m_points;
  smtk::mesh::TypeSet m_types;

  std::vector<smtk::mesh::Domain> m_domains;
  std::vector<smtk::mesh::Dirichlet> m_dirichlets;
  std::vector<smtk::mesh::Neumann> m_neumanns;
  smtk::common::UUIDArray m_uuids;
};

}
}
}


#endif
