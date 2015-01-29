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
#ifndef __smtk_mesh_moab_Interface_h
#define __smtk_mesh_moab_Interface_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/Interface.h"
#include "smtk/mesh/CellTypes.h"
#include "smtk/mesh/DimensionTypes.h"
#include "smtk/mesh/Handle.h"
#include "smtk/mesh/TypeSet.h"

#include <vector>

namespace moab
{
  class Interface;
}

namespace smtk {
namespace mesh {
namespace moab
{
//construct an empty interface instance, this is properly connected
//to a moab database
//----------------------------------------------------------------------------
SMTKCORE_EXPORT
smtk::mesh::moab::InterfacePtr make_interface();

//Given a smtk::mesh Collection extract the underlying smtk::mesh::moab interface
//from it. This requires that the collection was created with the proper interface
//to begin with.
//----------------------------------------------------------------------------
smtk::mesh::moab::InterfacePtr extract_interface( const smtk::mesh::CollectionPtr& c);

//Given a smtk::mesh Interface convert it to a smtk::mesh::moab interface, and than
//extract the raw moab interface pointer from that
//----------------------------------------------------------------------------
SMTKCORE_EXPORT
::moab::Interface* const extract_moab_interface( const smtk::mesh::InterfacePtr &iface);

//----------------------------------------------------------------------------
class SMTKCORE_EXPORT Interface : public smtk::mesh::Interface
{
public:
  Interface();

  virtual ~Interface();

  //----------------------------------------------------------------------------
  std::size_t numMeshes(smtk::mesh::Handle handle);

  //----------------------------------------------------------------------------
  //creates a meshset with no parent that contains the input cells.
  //this function needs to be expanded to support parenting
  //this function needs to be expanded to support adding tags to the meshset
  bool create_meshset(smtk::mesh::HandleRange cells,
                      smtk::mesh::Handle& meshHandle);

  //----------------------------------------------------------------------------
  smtk::mesh::Handle get_root();

  //----------------------------------------------------------------------------
  smtk::mesh::HandleRange get_meshsets(smtk::mesh::Handle handle);

  //----------------------------------------------------------------------------
  smtk::mesh::HandleRange get_meshsets(smtk::mesh::Handle handle,
                                       int dimension);

  //----------------------------------------------------------------------------
  //find all entity sets that have this exact name tag
  smtk::mesh::HandleRange get_meshsets(smtk::mesh::Handle handle,
                                       const std::string& name);

  //----------------------------------------------------------------------------
  //get all cells held by this range
  smtk::mesh::HandleRange get_cells(smtk::mesh::HandleRange meshsets);

  //----------------------------------------------------------------------------
  //get all cells held by this range handle of a given cell type
  smtk::mesh::HandleRange get_cells(smtk::mesh::HandleRange meshsets,
                                    smtk::mesh::CellType cellType);

  //----------------------------------------------------------------------------
  //get all cells held by this range handle of a given cell type(s)
  smtk::mesh::HandleRange get_cells(smtk::mesh::HandleRange meshsets,
                                    const smtk::mesh::CellTypes& cellTypes);

  //----------------------------------------------------------------------------
  //get all cells held by this range handle of a given dimension
  smtk::mesh::HandleRange get_cells(smtk::mesh::HandleRange meshsets,
                                    smtk::mesh::DimensionType dim);

  //----------------------------------------------------------------------------
  std::vector< std::string > compute_names(const smtk::mesh::HandleRange& r);

  //----------------------------------------------------------------------------
  smtk::mesh::TypeSet compute_types(smtk::mesh::Handle handle);

  //----------------------------------------------------------------------------
  smtk::mesh::HandleRange set_intersect(const smtk::mesh::HandleRange& a,
                                        const smtk::mesh::HandleRange& b) const;

  //----------------------------------------------------------------------------
  smtk::mesh::HandleRange set_difference(const smtk::mesh::HandleRange& a,
                                         const smtk::mesh::HandleRange& b) const;

  //----------------------------------------------------------------------------
  smtk::mesh::HandleRange set_union(const smtk::mesh::HandleRange& a,
                                    const smtk::mesh::HandleRange& b) const;

  //----------------------------------------------------------------------------
  smtk::mesh::HandleRange point_intersect(const smtk::mesh::HandleRange& a,
                                          const smtk::mesh::HandleRange& b,
                                          const smtk::mesh::ContainsFunctor& containsFunctor);
  //----------------------------------------------------------------------------
  smtk::mesh::HandleRange point_difference(const smtk::mesh::HandleRange& a,
                                           const smtk::mesh::HandleRange& b,
                                           const smtk::mesh::ContainsFunctor& containsFunctor);

  ::moab::Interface * const moabInterface() const;

private:
  //holds a reference to the real moab interface
  smtk::shared_ptr< ::moab::Interface > m_iface;

};

}
}
}

#endif
