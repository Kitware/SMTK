//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_utility_ExtractMeshConstants_h
#define __smtk_mesh_utility_ExtractMeshConstants_h

#include <cstdint>

#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/MeshSet.h"

namespace smtk
{
namespace model
{
class EntityRef;
class Loop;
} // namespace model
namespace mesh
{
namespace utility
{

class SMTKCORE_EXPORT PreAllocatedMeshConstants
{

public:
  static void determineAllocationLengths(
    const smtk::mesh::MeshSet& ms,
    std::int64_t& numberOfCells,
    std::int64_t& numberOfPoints);

  PreAllocatedMeshConstants(std::int64_t* cellMeshConstants, std::int64_t* pointMeshConstants);

private:
  template<typename QueryTag>
  friend SMTKCORE_EXPORT void extractMeshConstants(
    const smtk::mesh::MeshSet&,
    const smtk::mesh::PointSet&,
    PreAllocatedMeshConstants&);

  std::int64_t* m_cellMeshConstants;
  std::int64_t* m_pointMeshConstants;
};

class SMTKCORE_EXPORT MeshConstants
{
public:
  MeshConstants() = default;

  //This class self allocates all the memory needed to extract tessellation
  //and auto extract the tessellation based on the MeshSet you pass in
  void extractDirichlet(const smtk::mesh::MeshSet& ms);
  void extractNeumann(const smtk::mesh::MeshSet& ms);
  void extractDomain(const smtk::mesh::MeshSet& ms);

  void extractDirichlet(const smtk::mesh::MeshSet& cs, const smtk::mesh::PointSet& ps);
  void extractNeumann(const smtk::mesh::MeshSet& cs, const smtk::mesh::PointSet& ps);
  void extractDomain(const smtk::mesh::MeshSet& cs, const smtk::mesh::PointSet& ps);

  //use these methods to gain access to the field after extraction
  const std::vector<std::int64_t>& cellData() const { return m_cellData; }
  const std::vector<std::int64_t>& pointData() const { return m_pointData; }

private:
  template<typename QueryTag>
  void extract(const smtk::mesh::MeshSet& ms, const smtk::mesh::PointSet& ps);

  std::vector<std::int64_t> m_cellData;
  std::vector<std::int64_t> m_pointData;
};

//Don't wrap these for python, instead python should use the MeshConstants class and
//the extract method

SMTKCORE_EXPORT void extractDirichletMeshConstants(
  const smtk::mesh::MeshSet&,
  PreAllocatedMeshConstants&);
SMTKCORE_EXPORT void extractNeumannMeshConstants(
  const smtk::mesh::MeshSet&,
  PreAllocatedMeshConstants&);
SMTKCORE_EXPORT void extractDomainMeshConstants(
  const smtk::mesh::MeshSet&,
  PreAllocatedMeshConstants&);

//Extract MeshConstants with respect to another PointSet instead of the PointSet
//contained by the meshset. This is useful if you are sharing a single
//PointSet among multiple MeshConstantss.
SMTKCORE_EXPORT void extractDirichletMeshConstants(
  const smtk::mesh::MeshSet&,
  const smtk::mesh::PointSet&,
  PreAllocatedMeshConstants&);
SMTKCORE_EXPORT void extractNeumannMeshConstants(
  const smtk::mesh::MeshSet&,
  const smtk::mesh::PointSet&,
  PreAllocatedMeshConstants&);
SMTKCORE_EXPORT void extractDomainMeshConstants(
  const smtk::mesh::MeshSet&,
  const smtk::mesh::PointSet&,
  PreAllocatedMeshConstants&);

template<typename QueryTag>
// NOLINTNEXTLINE(readability-redundant-declaration)
SMTKCORE_EXPORT void extractMeshConstants(
  const smtk::mesh::MeshSet&,
  const smtk::mesh::PointSet&,
  PreAllocatedMeshConstants&);
} // namespace utility
} // namespace mesh
} // namespace smtk

#endif
