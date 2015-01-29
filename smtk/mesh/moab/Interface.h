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

#include "smtk/mesh/CellTypes.h"
#include "smtk/mesh/DimensionTypes.h"
#include "smtk/mesh/Handle.h"
#include "smtk/mesh/TypeSet.h"

#include "moab/Interface.hpp"

#include <vector>

namespace smtk {
namespace mesh {
namespace moab
{
//forward declare classes we use
struct ContainsFunctor;

//Interface is defined in PublicPointerDefs as a typedef to moab::interface
//We don't inherit from moab::interface since it is an abstract class
//Requires the CollectionPtr to not be NULL
//----------------------------------------------------------------------------
SMTKCORE_EXPORT
const smtk::mesh::moab::InterfacePtr& extractInterface(smtk::mesh::CollectionPtr c);

//construct an empty interface instance, this is properly connected
//to a moab database
//----------------------------------------------------------------------------
smtk::mesh::moab::InterfacePtr make_interface();

//----------------------------------------------------------------------------
SMTKCORE_EXPORT
std::size_t numMeshes(smtk::mesh::Handle handle,
                      const smtk::mesh::moab::InterfacePtr& iface);

//----------------------------------------------------------------------------
//creates a meshset with no parent that contains the input cells.
//this function needs to be expanded to support parenting
//this function needs to be expanded to support adding tags to the meshset
SMTKCORE_EXPORT
bool create_meshset(smtk::mesh::HandleRange cells,
                    smtk::mesh::Handle& meshHandle,
                    const smtk::mesh::moab::InterfacePtr& iface);

//----------------------------------------------------------------------------
SMTKCORE_EXPORT
smtk::mesh::HandleRange get_meshsets(smtk::mesh::Handle handle,
                                     const smtk::mesh::moab::InterfacePtr& iface);

//----------------------------------------------------------------------------
SMTKCORE_EXPORT
smtk::mesh::HandleRange get_meshsets(smtk::mesh::Handle handle,
                                     int dimension,
                                     const smtk::mesh::moab::InterfacePtr& iface);

//----------------------------------------------------------------------------
//find all entity sets that have this exact name tag
SMTKCORE_EXPORT
smtk::mesh::HandleRange get_meshsets(smtk::mesh::Handle handle,
                                     const std::string& name,
                                     const smtk::mesh::moab::InterfacePtr& iface);

//----------------------------------------------------------------------------
//get all cells held by this range
SMTKCORE_EXPORT
smtk::mesh::HandleRange get_cells(smtk::mesh::HandleRange meshsets,
                                  const smtk::mesh::moab::InterfacePtr& iface);

//----------------------------------------------------------------------------
//get all cells held by this range handle of a given cell type
SMTKCORE_EXPORT
smtk::mesh::HandleRange get_cells(smtk::mesh::HandleRange meshsets,
                                  smtk::mesh::CellType cellType,
                                  const smtk::mesh::moab::InterfacePtr& iface);

//----------------------------------------------------------------------------
//get all cells held by this range handle of a given cell type(s)
SMTKCORE_EXPORT
smtk::mesh::HandleRange get_cells(smtk::mesh::HandleRange meshsets,
                                  const smtk::mesh::CellTypes& cellTypes,
                                  const smtk::mesh::moab::InterfacePtr& iface);

//----------------------------------------------------------------------------
//get all cells held by this range handle of a given dimension
SMTKCORE_EXPORT
smtk::mesh::HandleRange get_cells(smtk::mesh::HandleRange meshsets,
                                  smtk::mesh::DimensionType dim,
                                  const smtk::mesh::moab::InterfacePtr& iface);

//----------------------------------------------------------------------------
SMTKCORE_EXPORT
std::vector< std::string > compute_names(const smtk::mesh::HandleRange& r,
                                         const smtk::mesh::moab::InterfacePtr& iface);

//----------------------------------------------------------------------------
SMTKCORE_EXPORT
smtk::mesh::TypeSet compute_types(smtk::mesh::Handle handle,
                                  const smtk::mesh::moab::InterfacePtr& iface);

//----------------------------------------------------------------------------
SMTKCORE_EXPORT
smtk::mesh::HandleRange point_intersect(const smtk::mesh::HandleRange& a,
                                        const smtk::mesh::HandleRange& b,
                                        const smtk::mesh::moab::ContainsFunctor& containsFunctor,
                                        const smtk::mesh::moab::InterfacePtr& iface);
//----------------------------------------------------------------------------
SMTKCORE_EXPORT
smtk::mesh::HandleRange point_difference(const smtk::mesh::HandleRange& a,
                                         const smtk::mesh::HandleRange& b,
                                         const smtk::mesh::moab::ContainsFunctor& containsFunctor,
                                         const smtk::mesh::moab::InterfacePtr& iface);


}
}
}

#endif
