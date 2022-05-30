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

#include "smtk/model/EntityIterator.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/json/jsonResource.h"

#include "smtk/io/ModelToMesh.h"

#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"

#include <fstream>

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

  smtk::mesh::CellSet cells(smtk::mesh::ResourcePtr mr) const
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

    typedef smtk::mesh::HandleRange::const_iterator c_it;
    std::size_t offset = 0;
    for (c_it i = pointIds.begin(); i != pointIds.end(); ++i)
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

void create_simple_mesh_model(smtk::model::ResourcePtr resource)
{
  std::string file_path(data_root);
  file_path += "/model/2d/smtk/test2D.json";

  std::ifstream file(file_path.c_str());

  std::string json_str((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
  nlohmann::json json = nlohmann::json::parse(json_str);

  smtk::model::from_json(json, resource);
  for (auto& tessPair : json["tessellations"])
  {
    smtk::common::UUID id = tessPair[0];
    smtk::model::Tessellation tess = tessPair[1];
    resource->setTessellation(id, tess);
  }

  resource->assignDefaultNames();

  file.close();
}

void verify_alloc_lengths_entityref(
  const smtk::model::EntityRef& eRef,
  const smtk::mesh::ResourcePtr& mr)
{

  smtk::mesh::MeshSet mesh = mr->findAssociatedMeshes(eRef);

  std::int64_t connectivityLength = -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
    eRef, mr, connectivityLength, numberOfCells, numberOfPoints);

  test(connectivityLength != -1);
  test(numberOfCells != -1);
  test(numberOfPoints != -1);

  test(static_cast<std::size_t>(connectivityLength) == mesh.pointConnectivity().size());
  test(static_cast<std::size_t>(numberOfCells) == mesh.cells().size());
  test(static_cast<std::size_t>(numberOfPoints) == mesh.points().size());
}

void verify_extract(const smtk::model::EntityRef& eRef, const smtk::mesh::ResourcePtr& mr)
{
  std::int64_t connectivityLength = -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
    eRef, mr, connectivityLength, numberOfCells, numberOfPoints);

  std::vector<std::int64_t> conn(connectivityLength);
  std::vector<float> fpoints(numberOfPoints * 3);

  smtk::mesh::utility::PreAllocatedTessellation ftess(conn.data(), fpoints.data());

  ftess.disableVTKStyleConnectivity(true);
  ftess.disableVTKCellTypes(true);
  smtk::mesh::utility::extractTessellation(eRef, mr, ftess);

  //lets iterate the points and make sure they all match
  smtk::mesh::CellSet cells = mr->findAssociatedCells(eRef);
  VerifyPoints<float> vp(fpoints);
  smtk::mesh::for_each(cells.points(), vp);
}

void verify_extract_volume_meshes_by_global_points_to_vtk(
  const smtk::model::EntityRef& eRef,
  const smtk::mesh::ResourcePtr& mr)
{
  smtk::mesh::CellSet cells = mr->findAssociatedCells(eRef);

  std::int64_t connectivityLength = -1;
  std::int64_t numberOfCells = -1;
  std::int64_t numberOfPoints = -1;

  //query for all cells
  smtk::mesh::utility::PreAllocatedTessellation::determineAllocationLengths(
    eRef, mr, connectivityLength, numberOfCells, numberOfPoints);

  std::vector<std::int64_t> conn(connectivityLength + numberOfCells);
  std::vector<std::int64_t> locations(numberOfCells);
  std::vector<unsigned char> types(numberOfCells);

  smtk::mesh::utility::PreAllocatedTessellation tess(conn.data(), locations.data(), types.data());

  //extract in releation to the points of all the meshes
  smtk::mesh::utility::extractTessellation(eRef, mr, mr->points(), tess);

  // //lets iterate the cells, and verify that the extraction matches
  // //what we see when we iterate
  VerifyCells vc(mr->cells(), conn, locations, types, true);
  smtk::mesh::for_each(cells, vc);
  test(vc.cells(mr) == cells);
}

void removeOnesWithoutTess(smtk::model::EntityRefs& ents)
{
  smtk::model::EntityIterator it;
  it.traverse(ents.begin(), ents.end(), smtk::model::ITERATE_BARE);
  std::vector<smtk::model::EntityRef> withoutTess;
  for (it.begin(); !it.isAtEnd(); ++it)
  {
    if (!it->hasTessellation())
    {
      withoutTess.push_back(it.current());
    }
  }

  typedef std::vector<smtk::model::EntityRef>::const_iterator c_it;
  for (c_it i = withoutTess.begin(); i < withoutTess.end(); ++i)
  {
    ents.erase(*i);
  }
}
} // namespace

int UnitTestExtractTessellationOfModel(int /*unused*/, char** const /*unused*/)
{
  // Somehow grab an EntityRef with an associated tessellation
  smtk::model::EntityRef eRef;
  smtk::model::ResourcePtr modelResource = smtk::model::Resource::create();

  create_simple_mesh_model(modelResource);

  smtk::io::ModelToMesh convert;
  convert.setIsMerging(false);
  smtk::mesh::ResourcePtr mr = convert(modelResource);

  typedef smtk::model::EntityRefs EntityRefs;
  typedef smtk::model::EntityTypeBits EntityTypeBits;

  EntityTypeBits etypes[4] = {
    smtk::model::VERTEX, smtk::model::EDGE, smtk::model::FACE, smtk::model::VOLUME
  };
  for (int i = 0; i != 4; ++i)
  {
    //extract all the coordinates from every tessellation and make a single
    //big pool
    EntityTypeBits entType = etypes[i];
    EntityRefs currentEnts = modelResource->entitiesMatchingFlagsAs<EntityRefs>(entType);
    removeOnesWithoutTess(currentEnts);
    if (!currentEnts.empty())
    {
      eRef = *currentEnts.begin();
      verify_alloc_lengths_entityref(eRef, mr);
      verify_extract(eRef, mr);
      verify_extract_volume_meshes_by_global_points_to_vtk(eRef, mr);
    }
  }

  return 0;
}
