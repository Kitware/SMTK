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

#include "smtk/mesh/utility/ExtractTessellation.h"

#include "smtk/io/ImportMesh.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

class VerifyCells : public smtk::mesh::CellForEach
{
  smtk::mesh::HandleRange m_cells;
  smtk::mesh::PointSet m_points;
  const std::vector<std::int64_t>& m_conn;
  const std::vector<std::int64_t>& m_locations;
  const std::vector<unsigned char>& m_types;

  std::size_t m_currentIndex{ 0 };
  std::int64_t m_currentLocation{ 0 };

  bool m_is_vtk;

public:
  VerifyCells(
    const smtk::mesh::CellSet& cells,
    const std::vector<std::int64_t>& conn,
    const std::vector<std::int64_t>& locations,
    const std::vector<unsigned char>& types,
    bool is_vtk_conn)
    : m_points(cells.points())
    , m_conn(conn)
    , m_locations(locations)
    , m_types(types)
    , m_is_vtk(is_vtk_conn)
  {
  }

  void forCell(const smtk::mesh::Handle& cellId, smtk::mesh::CellType cellType, int numPts) override
  {
    m_cells.insert(cellId);
    //verify the offset is in the correct location
    std::int64_t offset = m_locations[m_currentIndex];
    test(offset == m_currentLocation);
    if (m_is_vtk)
    {
      //the connectivity at offset should hold the number of points
      test(m_conn[offset] == numPts);
      m_currentLocation++;
      offset++;
    }
    else
    {
      //verify the types match when doing smtk types
      test(m_types[m_currentIndex] == static_cast<unsigned char>(cellType));
    }

    //verify the points ids are mapped properly
    for (int i = 0; i < numPts; ++i)
    {
      test(static_cast<std::size_t>(m_conn[offset + i]) == m_points.find(this->pointIds()[i]));
    }

    m_currentIndex++;
    m_currentLocation += numPts;
  }

  [[nodiscard]] smtk::mesh::CellSet cells(smtk::mesh::ResourcePtr mr) const
  {
    return smtk::mesh::CellSet(mr, m_cells);
  }
};

template<typename T>
class VerifyPoints : public smtk::mesh::PointForEach
{
  const std::vector<T>& m_points;
  std::size_t m_currentIndex{ 0 };

public:
  VerifyPoints(const std::vector<T>& points)
    : smtk::mesh::PointForEach()
    , m_points(points)
  {
  }

  void forPoints(
    const smtk::mesh::HandleRange& pointIds,
    std::vector<double>& xyz,
    bool& coordinatesModified) override
  {
    coordinatesModified = false; //we are not modifying the coords

    std::size_t offset = 0;
    for (auto i = smtk::mesh::rangeElementsBegin(pointIds);
         i != smtk::mesh::rangeElementsEnd(pointIds);
         ++i)
    {
      //iterate the range of coords / point ids
      test(m_points[m_currentIndex] == static_cast<T>(xyz[offset]));
      test(m_points[m_currentIndex + 1] == static_cast<T>(xyz[offset + 1]));
      test(m_points[m_currentIndex + 2] == static_cast<T>(xyz[offset + 2]));
      m_currentIndex += 3;
      offset += 3;
    }
  }
};

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

smtk::mesh::ResourcePtr load_mesh()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/sixth_hexflatcore.h5m";

  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  smtk::io::importMesh(file_path, mr);
  test(mr->isValid(), "resource should be valid");

  return mr;
}

void verify_constructors(const smtk::mesh::ResourcePtr& mr)
{

  smtk::mesh::MeshSet ms = mr->meshes();

  //construct tessellation object that only wants connectivity info
  {
    std::vector<std::int64_t> conn(1); //size doesn't matter right now

    smtk::mesh::utility::PreAllocatedTessellation tess(conn.data());

    test(tess.hasConnectivity());
    test(!tess.hasCellLocations());
    test(!tess.hasCellTypes());
    test(!tess.hasDoublePoints());
    test(!tess.hasFloatPoints());

    test(tess.useVTKConnectivity());
    tess.disableVTKStyleConnectivity(true);
    test(!tess.useVTKConnectivity());

    test(tess.useVTKCellTypes());
    tess.disableVTKCellTypes(true);
    test(!tess.useVTKCellTypes());
  }

  //construct tessellation object that only wants connectivity info and points
  {
    std::vector<std::int64_t> conn(1); //size doesn't matter right now
    std::vector<float> fpoints(1);     //size doesn't matter right now

    smtk::mesh::utility::PreAllocatedTessellation ftess(conn.data(), fpoints.data());

    test(ftess.hasConnectivity());
    test(!ftess.hasCellLocations());
    test(!ftess.hasCellTypes());
    test(!ftess.hasDoublePoints());
    test(ftess.hasFloatPoints());
    test(ftess.useVTKConnectivity());
    test(ftess.useVTKCellTypes());

    //now test with doubles
    std::vector<double> dpoints(1); //size doesn't matter right now
    smtk::mesh::utility::PreAllocatedTessellation dtess(conn.data(), dpoints.data());

    test(dtess.hasConnectivity());
    test(!dtess.hasCellLocations());
    test(!dtess.hasCellTypes());
    test(dtess.hasDoublePoints());
    test(!dtess.hasFloatPoints());
    test(dtess.useVTKConnectivity());
    test(dtess.useVTKCellTypes());
  }

  //construct tessellation object that only wants connectivity info, cell types
  //and cell locations bust doesnt want points
  {
    std::vector<std::int64_t> conn(1);      //size doesn't matter right now
    std::vector<std::int64_t> locations(1); //size doesn't matter right now
    std::vector<unsigned char> types(1);    //size doesn't matter right now

    smtk::mesh::utility::PreAllocatedTessellation tess(conn.data(), locations.data(), types.data());

    test(tess.hasConnectivity());
    test(tess.hasCellLocations());
    test(tess.hasCellTypes());
    test(!tess.hasDoublePoints());
    test(!tess.hasFloatPoints());
    test(tess.useVTKConnectivity());
    test(tess.useVTKCellTypes());
  }

  //construct tessellation object that wants everything
  {
    std::vector<std::int64_t> conn(1);      //size doesn't matter right now
    std::vector<std::int64_t> locations(1); //size doesn't matter right now
    std::vector<unsigned char> types(1);    //size doesn't matter right now
    std::vector<float> fpoints(1);          //size doesn't matter right now

    smtk::mesh::utility::PreAllocatedTessellation ftess(
      conn.data(), locations.data(), types.data(), fpoints.data());

    test(ftess.hasConnectivity());
    test(ftess.hasCellLocations());
    test(ftess.hasCellTypes());
    test(!ftess.hasDoublePoints());
    test(ftess.hasFloatPoints());
    test(ftess.useVTKConnectivity());
    test(ftess.useVTKCellTypes());

    //now test with doubles
    std::vector<double> dpoints(1); //size doesn't matter right now
    smtk::mesh::utility::PreAllocatedTessellation dtess(
      conn.data(), locations.data(), types.data(), dpoints.data());

    test(dtess.hasConnectivity());
    test(dtess.hasCellLocations());
    test(dtess.hasCellTypes());
    test(dtess.hasDoublePoints());
    test(!dtess.hasFloatPoints());
    test(dtess.useVTKConnectivity());
    test(dtess.useVTKCellTypes());
  }
}

void verify_alloc_lengths_meshset(const smtk::mesh::ResourcePtr& mr)
{

  smtk::mesh::MeshSet all_meshes = mr->meshes();
  smtk::mesh::MeshSet mesh3d = mr->meshes(smtk::mesh::Dims3);
  smtk::mesh::MeshSet mesh2d = mr->meshes(smtk::mesh::Dims2);

  std::int64_t connectivityLength = -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
    all_meshes, connectivityLength, numberOfCells, numberOfPoints);

  test(connectivityLength != -1);
  test(numberOfCells != -1);
  test(numberOfPoints != -1);

  test(static_cast<std::size_t>(connectivityLength) == all_meshes.pointConnectivity().size());
  test(static_cast<std::size_t>(numberOfCells) == all_meshes.cells().size());
  test(static_cast<std::size_t>(numberOfPoints) == all_meshes.points().size());

  //Now try asking only for 3d cells
  smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
    mesh3d, connectivityLength, numberOfCells, numberOfPoints);

  test(static_cast<std::size_t>(connectivityLength) == mesh3d.pointConnectivity().size());
  test(static_cast<std::size_t>(numberOfCells) == mesh3d.cells().size());
  test(static_cast<std::size_t>(numberOfPoints) == mesh3d.points().size());

  //Now try asking only for 2d cells
  smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
    mesh2d, connectivityLength, numberOfCells, numberOfPoints);

  test(static_cast<std::size_t>(connectivityLength) == mesh2d.pointConnectivity().size());
  test(static_cast<std::size_t>(numberOfCells) == mesh2d.cells().size());
  test(static_cast<std::size_t>(numberOfPoints) == mesh2d.points().size());
}

void verify_alloc_lengths_cellset(const smtk::mesh::ResourcePtr& mr)
{

  smtk::mesh::CellSet all_cells = mr->cells();
  smtk::mesh::CellSet cells3d = mr->cells(smtk::mesh::Dims3);
  smtk::mesh::CellSet cells2d = mr->cells(smtk::mesh::Dims2);

  std::int64_t connectivityLength = -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
    all_cells, connectivityLength, numberOfCells, numberOfPoints);

  test(connectivityLength != -1);
  test(numberOfCells != -1);
  test(numberOfPoints != -1);

  test(static_cast<std::size_t>(connectivityLength) == all_cells.pointConnectivity().size());
  test(static_cast<std::size_t>(numberOfCells) == all_cells.size());
  test(static_cast<std::size_t>(numberOfPoints) == all_cells.points().size());

  //Now try asking only for 3d cells
  smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
    cells3d, connectivityLength, numberOfCells, numberOfPoints);

  test(static_cast<std::size_t>(connectivityLength) == cells3d.pointConnectivity().size());
  test(static_cast<std::size_t>(numberOfCells) == cells3d.size());
  test(static_cast<std::size_t>(numberOfPoints) == cells3d.points().size());

  //Now try asking only for 2d cells
  smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
    cells2d, connectivityLength, numberOfCells, numberOfPoints);

  test(static_cast<std::size_t>(connectivityLength) == cells2d.pointConnectivity().size());
  test(static_cast<std::size_t>(numberOfCells) == cells2d.size());
  test(static_cast<std::size_t>(numberOfPoints) == cells2d.points().size());
}

void verify_extract_packed_single_type(const smtk::mesh::ResourcePtr& mr)
{

  smtk::mesh::MeshSet all_meshes = mr->meshes();
  smtk::mesh::CellSet quads = mr->cells(smtk::mesh::Quad);

  std::int64_t connectivityLength = -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
    quads, connectivityLength, numberOfCells, numberOfPoints);

  std::vector<std::int64_t> conn(connectivityLength);
  std::vector<float> fpoints(numberOfPoints * 3);

  smtk::mesh::utility::PreAllocatedTessellation ftess(conn.data(), fpoints.data());

  ftess.disableVTKStyleConnectivity(true);
  ftess.disableVTKCellTypes(true);
  // smtk::mesh::utility::extractTessellation(quads, ftess);
  smtk::mesh::utility::extractTessellation(quads, ftess);

  //lets iterate the points and make sure they all match
  VerifyPoints<float> vp(fpoints);
  smtk::mesh::for_each(quads.points(), vp);
}

void verify_extract_only_connectivity_and_types(const smtk::mesh::ResourcePtr& mr)
{
  smtk::mesh::CellSet cells3d = mr->cells(smtk::mesh::Dims3);

  std::int64_t connectivityLength = -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
    cells3d, connectivityLength, numberOfCells, numberOfPoints);

  std::vector<std::int64_t> conn(connectivityLength);
  std::vector<std::int64_t> locations(numberOfCells);
  std::vector<unsigned char> types(numberOfCells);

  smtk::mesh::utility::PreAllocatedTessellation tess(conn.data(), locations.data(), types.data());

  tess.disableVTKStyleConnectivity(true);
  tess.disableVTKCellTypes(true);
  smtk::mesh::utility::extractTessellation(cells3d, tess);

  //lets iterate the cells, and verify that the extraction matches
  //what we see when we iterate
  VerifyCells vc(cells3d, conn, locations, types, false);
  smtk::mesh::for_each(cells3d, vc);
  test(vc.cells(mr) == cells3d);
}

void verify_extract_all_to_vtk(const smtk::mesh::ResourcePtr& mr)
{
  smtk::mesh::CellSet cells3d = mr->cells(smtk::mesh::Dims3);

  std::int64_t connectivityLength = -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
    cells3d, connectivityLength, numberOfCells, numberOfPoints);

  std::vector<std::int64_t> conn(connectivityLength + numberOfCells);
  std::vector<std::int64_t> locations(numberOfCells);
  std::vector<unsigned char> types(numberOfCells);
  std::vector<double> dpoints(numberOfPoints * 3);

  smtk::mesh::utility::PreAllocatedTessellation tess(
    conn.data(), locations.data(), types.data(), dpoints.data());

  smtk::mesh::utility::extractTessellation(cells3d, tess);

  //lets iterate the cells, and verify that the extraction matches
  //what we see when we iterate
  VerifyCells vc(cells3d, conn, locations, types, true);
  smtk::mesh::for_each(cells3d, vc);
  test(vc.cells(mr) == cells3d);

  //lets iterate the points and make sure they all match
  VerifyPoints<double> vp(dpoints);
  smtk::mesh::for_each(cells3d.points(), vp);
}

void verify_extract_only_connectivity_to_vtk(const smtk::mesh::ResourcePtr& mr)
{
  smtk::mesh::CellSet cells2d = mr->cells(smtk::mesh::Dims2);

  std::int64_t connectivityLength = -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
    cells2d, connectivityLength, numberOfCells, numberOfPoints);

  std::vector<std::int64_t> conn(connectivityLength + numberOfCells);
  std::vector<std::int64_t> locations(numberOfCells);
  std::vector<unsigned char> types(numberOfCells);

  smtk::mesh::utility::PreAllocatedTessellation tess(conn.data(), locations.data(), types.data());

  smtk::mesh::utility::extractTessellation(cells2d, tess);

  //lets iterate the cells, and verify that the extraction matches
  //what we see when we iterate
  VerifyCells vc(cells2d, conn, locations, types, true);
  smtk::mesh::for_each(cells2d, vc);
  test(vc.cells(mr) == cells2d);
}

void verify_extract_volume_meshes_by_global_points_to_vtk(const smtk::mesh::ResourcePtr& mr)
{
  smtk::mesh::CellSet cells = mr->cells(smtk::mesh::Dims1);
  cells.append(mr->cells(smtk::mesh::Dims2));

  std::int64_t connectivityLength = -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
    cells, connectivityLength, numberOfCells, numberOfPoints);

  std::vector<std::int64_t> conn(connectivityLength + numberOfCells);
  std::vector<std::int64_t> locations(numberOfCells);
  std::vector<unsigned char> types(numberOfCells);

  smtk::mesh::utility::PreAllocatedTessellation tess(conn.data(), locations.data(), types.data());

  //extract in releation to the points of all the meshes
  smtk::mesh::utility::extractTessellation(cells, mr->points(), tess);

  // //lets iterate the cells, and verify that the extraction matches
  // //what we see when we iterate
  VerifyCells vc(mr->cells(), conn, locations, types, true);
  smtk::mesh::for_each(cells, vc);
  test(vc.cells(mr) == cells);
}
} // namespace

int UnitTestExtractTessellation(int /*unused*/, char** const /*unused*/)
{
  smtk::mesh::ResourcePtr mr = load_mesh();

  verify_constructors(mr);

  verify_alloc_lengths_meshset(mr);
  verify_alloc_lengths_cellset(mr);

  verify_extract_packed_single_type(mr);
  verify_extract_only_connectivity_and_types(mr);

  verify_extract_all_to_vtk(mr);
  verify_extract_only_connectivity_to_vtk(mr);

  verify_extract_volume_meshes_by_global_points_to_vtk(mr);

  return 0;
}
