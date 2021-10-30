//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_mesh_utility_ApplyToMesh_h
#define smtk_mesh_utility_ApplyToMesh_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/core/MeshSet.h"

#include <string>

namespace smtk
{
namespace mesh
{
namespace utility
{

// deform each point in a meshset according to an R^3->R^3 mapping.
SMTKCORE_EXPORT
bool applyWarp(
  const std::function<std::array<double, 3>(std::array<double, 3>)>&,
  smtk::mesh::MeshSet& ms,
  bool storePriorCoordinates = false);

// if prior coordinates were stored during applyWarp, undoWarp resets the
// coordinates to their original values.
SMTKCORE_EXPORT
bool undoWarp(smtk::mesh::MeshSet& ms);

// construct a named scalar field defined at each point in a meshset according
// to an R^3->R mapping.
SMTKCORE_EXPORT
bool applyScalarPointField(
  const std::function<double(std::array<double, 3>)>&,
  const std::string& name,
  smtk::mesh::MeshSet& ms);

// construct a named scalar field defined at each cell centroid in a meshset
// according to an R^3->R mapping.
SMTKCORE_EXPORT
bool applyScalarCellField(
  const std::function<double(std::array<double, 3>)>&,
  const std::string& name,
  smtk::mesh::MeshSet& ms);

// construct a named vector field defined at each point in a meshset according
// to an R^3->R^3 mapping.
SMTKCORE_EXPORT
bool applyVectorPointField(
  const std::function<std::array<double, 3>(std::array<double, 3>)>&,
  const std::string& name,
  smtk::mesh::MeshSet& ms);

// construct a named vector field defined at each cell centroid in a meshset
// according to an R^3->R^3 mapping.
SMTKCORE_EXPORT
bool applyVectorCellField(
  const std::function<std::array<double, 3>(std::array<double, 3>)>&,
  const std::string& name,
  smtk::mesh::MeshSet& ms);
} // namespace utility
} // namespace mesh
} // namespace smtk

#endif
