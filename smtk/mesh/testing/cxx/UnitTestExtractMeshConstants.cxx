//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/utility/ExtractMeshConstants.h"

#include "smtk/io/ReadMesh.h"

#include "smtk/mesh/testing/cxx/helpers.h"

#include <limits>

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

std::string mesh_path()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/64bricks_12ktet.h5m";
  return file_path;
}

class SubdivideMesh : public smtk::mesh::MeshForEach
{
public:
  SubdivideMesh(const std::vector<std::size_t>& order)
    : m_order(order)
  {
  }

  void forMesh(smtk::mesh::MeshSet& mesh) override
  {
    std::size_t i = m_index++ % m_order.size();
    if (i != 0)
    {
      mesh.setDomain(smtk::mesh::Domain(static_cast<int>(i)));
    }
  }

private:
  const std::vector<std::size_t>& m_order;
  std::size_t m_index{ 0 };
};

class ValidateCells : public smtk::mesh::CellForEach
{
public:
  ValidateCells(
    const std::vector<smtk::mesh::CellSet>& cellsByDomain,
    const std::vector<smtk::mesh::Domain>& domains,
    const std::int64_t* domainAssignments,
    const smtk::mesh::HandleRange cellRange)
    : m_cellsByDomain(cellsByDomain)
    , m_domains(domains)
    , m_domainAssignments(domainAssignments)
    , m_cellRange(cellRange)
  {
  }

  void forCell(
    const smtk::mesh::Handle& cellId,
    smtk::mesh::CellType /*cellType*/,
    int /*numPointIds*/) override
  {
    // default to the value for unlabeled domains
    std::int64_t domainValue = -1;

    for (std::size_t i = 0; i < m_cellsByDomain.size(); ++i)
    {
      if (m_cellsByDomain[i].range().find(cellId) != m_cellsByDomain[i].range().end())
      {
        domainValue = m_domains[i].value();
        break;
      }
    }

    test(m_domainAssignments[m_index++] == domainValue);
  }

  const std::vector<smtk::mesh::CellSet>& m_cellsByDomain;
  const std::vector<smtk::mesh::Domain>& m_domains;
  const std::int64_t* m_domainAssignments;
  const smtk::mesh::HandleRange m_cellRange;
  std::size_t m_index{ 0 };
};

class ValidatePoints : public smtk::mesh::PointForEach
{
public:
  ValidatePoints(
    const std::vector<smtk::mesh::PointSet>& pointsByDomain,
    const std::vector<smtk::mesh::Domain>& domains,
    const std::int64_t* domainAssignments,
    const smtk::mesh::HandleRange pointRange)
    : m_pointsByDomain(pointsByDomain)
    , m_domains(domains)
    , m_domainAssignments(domainAssignments)
    , m_pointRange(pointRange)
  {
  }

  void forPoints(
    const smtk::mesh::HandleRange& pointIds,
    std::vector<double>& /*xyz*/,
    bool& /*coordinatesModified*/) override
  {
    for (auto point = pointIds.begin(); point != pointIds.end(); ++point)
    {
      // default to the value for unlabeled domains
      std::int64_t domainValue = -1;

      for (std::size_t i = 0; i < m_pointsByDomain.size(); ++i)
      {
        if (m_pointsByDomain[i].range().find(*point) != m_pointsByDomain[i].range().end())
        {
          domainValue = m_domains[i].value();
          break;
        }
      }
      test(m_domainAssignments[m_index++] == domainValue);
    }
  }

  const std::vector<smtk::mesh::PointSet>& m_pointsByDomain;
  const std::vector<smtk::mesh::Domain>& m_domains;
  const std::int64_t* m_domainAssignments;
  const smtk::mesh::HandleRange m_pointRange;
  std::size_t m_index{ 0 };
};

void verify_extract_domain()
{
  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  read(mesh_path(), mr, smtk::io::mesh::Subset::OnlyNeumann);
  test(mr->isValid(), "resource should be valid");

  smtk::mesh::MeshSet meshes = mr->meshes();

  test(!meshes.is_empty());

  // some random order to assign domains
  std::vector<std::size_t> order = { 1, 2, 3, 4, 3, 5 };

  SubdivideMesh subdivideMesh(order);
  smtk::mesh::for_each(meshes, subdivideMesh);

  std::int64_t numberOfCells;
  std::int64_t numberOfPoints;

  smtk::mesh::utility::PreAllocatedMeshConstants::determineAllocationLengths(
    meshes, numberOfCells, numberOfPoints);

  std::int64_t* cells = new std::int64_t[numberOfCells];
  std::int64_t* points = new std::int64_t[numberOfPoints];

  smtk::mesh::utility::PreAllocatedMeshConstants meshConstants(cells, points);
  smtk::mesh::utility::extractDomainMeshConstants(meshes, meshConstants);

  std::vector<smtk::mesh::Domain> domains = meshes.domains();
  std::vector<smtk::mesh::CellSet> cellsByDomain;
  std::vector<smtk::mesh::PointSet> pointsByDomain;
  for (auto&& domain : domains)
  {
    cellsByDomain.push_back(meshes.subset(domain).cells());
    pointsByDomain.push_back(meshes.subset(domain).points());
  }

  ValidateCells validateCells(cellsByDomain, domains, cells, meshes.cells().range());
  smtk::mesh::for_each(meshes.cells(), validateCells);

  ValidatePoints validatePoints(pointsByDomain, domains, points, meshes.points().range());
  smtk::mesh::for_each(meshes.points(), validatePoints);

  delete[] cells;
  delete[] points;
}
} // namespace

int UnitTestExtractMeshConstants(int /*unused*/, char** const /*unused*/)
{
  verify_extract_domain();

  return 0;
}
