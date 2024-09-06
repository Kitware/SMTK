//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/mesh/MeshIOXMS.h"
#include "smtk/io/mesh/MeshIO.h"

#include "smtk/io/Logger.h"

#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/CellTypes.h"
#include "smtk/mesh/core/DimensionTypes.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/utility/ExtractTessellation.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Resource.h"

#include "smtk/Regex.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#define BOOST_FILESYSTEM_VERSION 3
#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>

namespace smtk
{
namespace io
{
namespace mesh
{

namespace
{

struct OpenFile
{
  OpenFile(const std::string& path)
    : m_stream(path.c_str(), std::ios::out)
  {
    m_path = path;
    m_canWrite = m_stream.is_open(); //verify file is open
    m_deleteFile = true;
  }

  ~OpenFile()
  {
    if (m_canWrite)
    {
      m_stream.close();

      if (m_deleteFile)
      {
        std::remove(m_path.c_str());
      }
    }
  }

  void fileWritten(bool written)
  {
    //If the file was written, we don't need to delete it
    m_deleteFile = !written;
  }

  std::string m_path;
  std::fstream m_stream;
  bool m_canWrite;
  bool m_deleteFile;
};

struct MeshByRegion
{
  MeshByRegion(const smtk::mesh::MeshSet& ms, int regionId, smtk::mesh::DimensionType dim)
    : m_meshSet(ms)
    , m_dim(dim)
    , m_regionId(regionId)
  {
  }

  int region() const { return m_regionId; }

  //return number cells of the set dimension
  std::size_t numCells() const { return m_meshSet.cells(m_dim).size(); }

  //return all the cells of a given type
  smtk::mesh::CellSet cells(const smtk::mesh::CellType& type) const
  {
    return m_meshSet.cells(type);
  }

  //return all the cells, restricted to the dimension passed in during
  //the constructor
  smtk::mesh::CellSet cells() const { return m_meshSet.cells(m_dim); }

  smtk::mesh::MeshSet m_meshSet;
  smtk::mesh::DimensionType m_dim;
  int m_regionId;
};

template<typename T, typename U>
double find_sum(const U& conn, const T& points, std::size_t index, int nVerts)
{
  double sum = 0;
  for (int j = 0; j < nVerts; ++j)
  {
    std::size_t c1 = conn[index + j];
    std::size_t c2 = conn[index + ((j + 1) % nVerts)];

    double x1 = points[c1 * 3];
    double y1 = points[c1 * 3 + 1];

    double x2 = points[c2 * 3];
    double y2 = points[c2 * 3 + 1];

    sum += (x2 - x1) * (y2 + y1);
  }
  return sum;
}

std::string to_CardType(smtk::mesh::CellType type)
{

  if (type == smtk::mesh::Line)
  {
    return std::string("E2L");
  }
  if (type == smtk::mesh::Triangle)
  {
    return std::string("E3T");
  }
  else if (type == smtk::mesh::Quad)
  {
    return std::string("E4Q");
  }
  else if (type == smtk::mesh::Tetrahedron)
  {
    return std::string("E4T");
  }
  else if (type == smtk::mesh::Pyramid)
  {
    return std::string("E5P");
  }
  else if (type == smtk::mesh::Wedge)
  {
    return std::string("E6W");
  }
  else if (type == smtk::mesh::Hexahedron)
  {
    return std::string("E8H");
  }
  return std::string("B4D");
}

class WriteCellsPerRegion
{
  smtk::mesh::PointSet m_PointSet;
  std::ostream& m_Stream;
  int m_CellId{ 1 }; //2dm/3dm requires id values to start at 1

public:
  WriteCellsPerRegion(const smtk::mesh::PointSet& ps, std::ostream& stream)
    : m_PointSet(ps)
    , m_Stream(stream)
  {
  }

  void operator()(const MeshByRegion& mbr, const smtk::mesh::CellType& type)
  {
    smtk::mesh::CellSet cells = mbr.cells(type);

    //If we have zero cells, don't do anything
    if (cells.is_empty())
    {
      return;
    }

    const int regionId = mbr.region();
    const int nVerts = smtk::mesh::verticesPerCell(type);
    std::string cardType = to_CardType(type);

    if (type == smtk::mesh::Triangle || type == smtk::mesh::Quad)
    {
      this->writeCounterClockwise(cells, cardType, regionId, nVerts);
    }
    else
    {
      this->write(cells, cardType, regionId, nVerts);
    }
  }

  void
  write(const smtk::mesh::CellSet& cells, const std::string& cardType, int regionId, int nVerts)
  {
    //Use the extractTessellation helpers to convert the connectivity
    //to map properly to the PointSet that represents ALL points we are
    //using, not just the points these cells are using
    std::int64_t connectivityLen = cells.pointConnectivity().size();
    std::vector<std::int64_t> conn(connectivityLen);

    smtk::mesh::utility::PreAllocatedTessellation connectivityInfo(conn.data());
    connectivityInfo.disableVTKStyleConnectivity(true);
    smtk::mesh::utility::extractTessellation(cells, m_PointSet, connectivityInfo);

    //now we just need to write out the cells
    std::size_t nCells = cells.size();
    for (std::size_t i = 0; i < nCells; ++i)
    {
      m_Stream << cardType << " \t " << m_CellId++ << " ";
      for (int j = 0; j < nVerts; ++j)
      {
        //We add 1, since the points are written out starting with index 1
        m_Stream << std::setw(8) << 1 + conn[nVerts * i + j] << " ";
      }
      m_Stream << std::setw(8) << regionId << std::endl;
    }
  }

  void writeCounterClockwise(
    const smtk::mesh::CellSet& cells,
    const std::string& cardType,
    int regionId,
    int nVerts)
  {
    //Use the extractTessellation helpers to convert the connectivity
    //to map properly to the PointSet that represents ALL points we are
    //using, not just the points these cells are using
    std::int64_t connectivityLen = cells.pointConnectivity().size();
    std::int64_t pointLen = m_PointSet.size() * 3;

    std::vector<std::int64_t> conn(connectivityLen);
    std::vector<double> points(pointLen);

    smtk::mesh::utility::PreAllocatedTessellation connectivityInfo(conn.data(), points.data());
    connectivityInfo.disableVTKStyleConnectivity(true);
    smtk::mesh::utility::extractTessellation(cells, m_PointSet, connectivityInfo);

    //when writing out a triangle or quad region the cell must be written
    //in counter clockwise orientation. We are presuming that for 2d meshes
    //the triangles are all planar, so we can use the shoelace formula
    //to determine if the points are in clockwise order.
    // https://en.wikipedia.org/wiki/Shoelace_formula
    std::size_t nCells = cells.size();
    std::size_t cIndex = 0;
    for (std::size_t i = 0; i < nCells; ++i, cIndex += nVerts)
    {

      //determine if the triangle/quad is counterclockwise
      //a positive sum denotes a clockwise winding
      double sum = find_sum(conn, points, cIndex, nVerts);
      if (sum > 0)
      { //we have a clockwise cell that we need to reverse
        std::reverse(&conn[cIndex], &conn[cIndex + nVerts]);
      }

      //now that the connectivity is the correct order we can write it out
      m_Stream << cardType << " \t " << m_CellId++ << " ";
      for (int j = 0; j < nVerts; ++j)
      {
        //We add 1, since the points are written out starting with index 1
        m_Stream << std::setw(8) << 1 + conn[cIndex + j] << " ";
      }
      m_Stream << std::setw(8) << regionId << std::endl;
    }
  }
};

std::vector<MeshByRegion> subsetByRegion(
  smtk::mesh::ResourcePtr meshResource,
  smtk::mesh::DimensionType type)
{
  std::vector<MeshByRegion> meshesByModelRef;
  smtk::mesh::MeshSet meshes = meshResource->meshes(type);
  if (meshes.is_empty())
  { //if we have no meshes, stop
    return meshesByModelRef;
  }

  smtk::model::EntityRefArray modelIds;
  meshes.modelEntities(modelIds);

  if (modelIds.size() <= 1)
  { //explicitly done so that no model relationship is equal to everything
    //being in the same region
    meshesByModelRef.emplace_back(meshes, 1, type);
  }
  else
  {
    meshesByModelRef.reserve(modelIds.size());
    int region = 1;

    typedef smtk::model::EntityRefArray::const_iterator it;
    for (it i = modelIds.begin(); i != modelIds.end(); ++i)
    {
      smtk::mesh::MeshSet subset = meshResource->findAssociatedMeshes(*i, type);
      if (!subset.is_empty())
      {
        meshesByModelRef.emplace_back(subset, region, type);
        ++region;
      }
    }
  }
  return meshesByModelRef;
}

std::vector<MeshByRegion> subsetByModelProperty(
  smtk::mesh::ResourcePtr meshResource,
  smtk::model::ResourcePtr resource,
  const std::string& modelPropertyName,
  smtk::mesh::DimensionType type)
{
  std::vector<MeshByRegion> meshesByModelRef;
  smtk::mesh::MeshSet meshes = meshResource->meshes(type);
  if (meshes.is_empty())
  { //if we have no meshes, stop
    return meshesByModelRef;
  }

  smtk::model::EntityRefArray modelIds;
  meshes.modelEntities(modelIds);

  typedef smtk::model::EntityRefArray::const_iterator it;
  for (it i = modelIds.begin(); i != modelIds.end(); ++i)
  {
    const smtk::model::IntegerList& values =
      resource->integerProperty(i->entity(), modelPropertyName);
    smtk::mesh::MeshSet subset = meshResource->findAssociatedMeshes(*i, type);
    if (values.size() == 1 && !subset.is_empty())
    { //only accept model properties that have single values
      //since that is what we think region id's should be
      const int& region = values[0];
      meshesByModelRef.emplace_back(subset, region, type);
    }
  }

  return meshesByModelRef;
}

smtk::mesh::PointSet pointsUsed(const std::vector<MeshByRegion>& meshes)
{
  //make a single mesh that represents all the cells of a given dimension
  smtk::mesh::CellSet cs = meshes[0].cells();
  const std::size_t size = meshes.size();
  for (std::size_t i = 1; i < size; ++i)
  {
    cs.append(meshes[i].cells());
  }
  return cs.points();
}

bool write_dm(
  const std::vector<MeshByRegion>& meshes,
  std::ostream& stream,
  smtk::mesh::DimensionType type)
{
  smtk::mesh::PointSet pointSet = pointsUsed(meshes);

  //write the header block to the stream
  if (type == smtk::mesh::Dims1)
  {
    stream << "MESH1D" << std::endl;
  }
  else if (type == smtk::mesh::Dims2)
  {
    stream << "MESH2D" << std::endl;
  }
  else if (type == smtk::mesh::Dims3)
  {
    stream << "MESH3D" << std::endl;
  }
  else
  { //bad dimension bail!
    return false;
  }

  //write out some comment lines that contain
  //info on the number of cells and points
  const std::size_t numMeshes = meshes.size();
  std::size_t numPoints = pointSet.numberOfPoints();
  std::size_t numCells = 0;
  for (std::size_t i = 0; i < numMeshes; ++i)
  {
    numCells += meshes[i].numCells();
  }

  stream << "#NELEM " << numCells << std::endl;
  stream << "#NNODE " << numPoints << std::endl;

  //now that we have the meshes on a per region basis we can
  //start to dump them to file
  //we just need to:
  //Determine all the cells that will
  //1. Iterate the meshes
  //2. For each mesh subset by cell type
  //3. iterate the cells in each subtype
  //4. dump out the points
  WriteCellsPerRegion writer(pointSet, stream);
  for (std::size_t i = 0; i < numMeshes; ++i)
  {
    const MeshByRegion& mbr = meshes[i];
    //subset the cells of the requested dimension by type
    if (type == 2)
    {
      writer(mbr, smtk::mesh::Triangle);
      writer(mbr, smtk::mesh::Quad);
    }
    else
    { //we presume 3d
      writer(mbr, smtk::mesh::Tetrahedron);
      writer(mbr, smtk::mesh::Pyramid);
      writer(mbr, smtk::mesh::Wedge);
      writer(mbr, smtk::mesh::Hexahedron);
    }
  }

  //now that the cells have been written
  //we do step 4. For step 4 we manual extract
  //the points

  std::vector<double> xyz(numPoints * 3);
  pointSet.get(xyz.data()); //fill our buffer

  for (std::size_t i = 0; i < numPoints; ++i)
  {
    stream << "ND \t " << std::setw(8) << 1 + i << " " << std::fixed << std::setw(12) << xyz[3 * i]
           << " " << std::setw(12) << xyz[(3 * i) + 1] << " " << std::setw(12) << xyz[(3 * i) + 2]
           << std::endl;
  }

  return true;
}

bool write_dm(
  smtk::mesh::ResourcePtr meshResource,
  std::ostream& stream,
  smtk::mesh::DimensionType type)
{
  if (!meshResource)
  { //can't write out an empty mesh Resource
    return false;
  }

  std::vector<MeshByRegion> meshes = subsetByRegion(meshResource, type);
  if (meshes.empty())
  { //nothing to write out
    return false;
  }

  return write_dm(meshes, stream, type);
}

bool write_dm(
  smtk::mesh::ResourcePtr meshResource,
  smtk::model::ResourcePtr resource,
  const std::string& modelPropertyName,
  std::ostream& stream,
  smtk::mesh::DimensionType type)
{
  if (!meshResource)
  { //can't write out an empty mesh Resource
    return false;
  }

  std::vector<MeshByRegion> meshes =
    subsetByModelProperty(meshResource, resource, modelPropertyName, type);
  if (meshes.empty())
  { //nothing to write out
    return false;
  }

  return write_dm(meshes, stream, type);
}

std::size_t computeNumberOfPoints(std::istream& stream)
{
  std::size_t nPts = 0;
  std::size_t counter = 0;
  bool fromComment = false;

  smtk::regex re("\\s+");

  std::string line;
  while (std::getline(stream, line))
  {
    // .*dm files often have a commented out "NNODE" field. Other readers seem
    // to key off of this commented value, so we do the same (even though we
    // could just count nodes instead of depending on comment strings).

    // passing -1 as the submatch index parameter performs splitting
    smtk::sregex_token_iterator first{ line.begin(), line.end(), re, -1 };
    if (*first == "#NNODE")
    {
      fromComment = true;
      nPts = std::stoul(*(++first));
      break;
    }
    else if (*first == "ND")
    {
      ++counter;
      std::size_t tmp = std::stoul(*(++first));
      nPts = (nPts < tmp ? tmp : nPts);
    }
  }

  // reset the stream to the beginning of the file
  stream.clear();
  stream.seekg(0, std::istream::beg);

  if (!fromComment && counter != nPts)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Unexpected number of points.");
  }
  assert(fromComment || counter == nPts);

  return nPts;
}

bool readPoints(std::istream& stream, const smtk::mesh::BufferedCellAllocatorPtr& bcAllocator)
{
  std::string line;

  std::size_t nPts = computeNumberOfPoints(stream);

  bcAllocator->reserveNumberOfCoordinates(nPts);

  smtk::regex re("\\s+");

  std::size_t index;
  double xyz[3];
  while (std::getline(stream, line))
  {
    // passing -1 as the submatch index parameter performs splitting
    smtk::sregex_token_iterator first{ line.begin(), line.end(), re, -1 }, last;
    if (*first == "ND")
    {
      // ensure that the file format is at least as long as we expect
      // (ND <index> <x> <y> <z>)
      if (std::distance(first, last) < 5)
      {
        std::cout << "ERROR: points should have at least 5 fields." << std::endl;
        return false;
      }

      // access the point index and shift it from 1-based to 0-based indexing
      index = std::stoul(*(++first)) - 1;

      // ensure that the index falls within the precomputed range of points
      assert(index < nPts);

      // access the x, y and z coordiantes
      for (std::size_t i = 0; i < 3; i++)
      {
        xyz[i] = std::stod(*(++first));
      }

      // set the coordinates
      bcAllocator->setCoordinate(index, xyz);
    }
  }

  // reset the stream to the beginning of the file
  stream.clear();
  stream.seekg(0, std::istream::beg);

  return true;
}

smtk::mesh::CellType to_CellType(const std::string& type)
{

  if (type == "E2L")
  {
    return smtk::mesh::Line;
  }
  if (type == "E3T")
  {
    return smtk::mesh::Triangle;
  }
  if (type == "E4Q")
  {
    return smtk::mesh::Quad;
  }
  if (type == "E4T")
  {
    return smtk::mesh::Tetrahedron;
  }
  if (type == "E5P")
  {
    return smtk::mesh::Pyramid;
  }
  if (type == "E6W")
  {
    return smtk::mesh::Wedge;
  }
  if (type == "E8H")
  {
    return smtk::mesh::Hexahedron;
  }
  return smtk::mesh::CellType_MAX;
}

// Given a mesh Resource with multiple meshes for each domain, condense the
// meshsets so there is only one meshset per domain.
void condenseMeshsetsByDomain(smtk::mesh::ResourcePtr& meshResource)
{
  auto domains = meshResource->domains();
  for (auto& domain : domains)
  {
    auto meshForDomain = meshResource->domainMeshes(domain);
    auto cellsForDomain = meshForDomain.cells();

    // construct a mesh set from these cells
    auto singleMeshForDomain = meshResource->createMesh(cellsForDomain);

    // set the domain on the mesh set
    meshResource->setDomainOnMeshes(singleMeshForDomain, domain);

    for (std::size_t i = 0; i < meshForDomain.size(); i++)
    {
      meshResource->removeMeshes(meshForDomain.subset(i));
    }
  }
}

bool readCells(
  std::istream& stream,
  const smtk::mesh::BufferedCellAllocatorPtr& bcAllocator,
  smtk::mesh::ResourcePtr& meshResource)
{
  smtk::regex re("\\s+");

  std::string line;
  std::vector<long long int> connectivity;
  smtk::mesh::HandleRange cellsWithMaterials = bcAllocator->cells();
  int currentMaterialId = -1;
  int materialId;

  while (std::getline(stream, line))
  {
    // passing -1 as the submatch index parameter performs splitting
    smtk::sregex_token_iterator first{ line.begin(), line.end(), re, -1 }, last;
    if (std::string(*first)[0] == 'E')
    {
      smtk::mesh::CellType type = to_CellType(*first);
      if (type == smtk::mesh::CellType_MAX)
      {
        // Have we reached an "END" string?
        if (*first == "END")
        {
          break;
        }
        std::cout << "ERROR: Unsupported cell type \"" << *first << "\"." << std::endl;
        return false;
      }

      std::size_t nVerticesPerCell = smtk::mesh::verticesPerCell(type);

      // ensure that the file format is at least as long as we expect
      // (E#X <index> <conn_1> <conn_2> ... <conn_n> <group>)
      typedef std::iterator_traits<smtk::sregex_token_iterator>::difference_type diff_type;
      if (std::distance(first, last) < static_cast<diff_type>(nVerticesPerCell + 3))
      {
        std::cout << "ERROR: cell type \"" << *first << "\" should have at least "
                  << nVerticesPerCell + 3 << " fields." << std::endl;
        return false;
      }

      connectivity.resize(nVerticesPerCell);

      // skip the cell index.
      ++first;

      // access the x, y and z coordiantes
      for (std::size_t i = 0; i < nVerticesPerCell; i++)
      {
        // access the point index and shift it from 1-based to 0-based indexing
        connectivity[i] = std::stol(*(++first)) - 1;
      }

      // access the cell's material id
      materialId = std::stoi(*(++first));

      // if it differs from the current material being parsed...
      if (materialId != currentMaterialId)
      {
        // ...and this is not the first material encountered...
        if (currentMaterialId != -1)
        {
          // ...flush the allocator
          bcAllocator->flush();

          // construct a cell set containing only the cells of this material type
          smtk::mesh::CellSet cellsForMaterial(
            meshResource, (bcAllocator->cells() - cellsWithMaterials));

          // construct a mesh set from these cells
          smtk::mesh::MeshSet meshForMaterial = meshResource->createMesh(cellsForMaterial);

          // set the domain on the mesh set
          meshResource->setDomainOnMeshes(meshForMaterial, smtk::mesh::Domain(currentMaterialId));
        }

        // update the material id
        currentMaterialId = materialId;

        // update the set of currently processed cells
        cellsWithMaterials = bcAllocator->cells();
      }

      // add the cell
      bcAllocator->addCell(type, connectivity.data());
    }
  }

  // flush the allocator
  bcAllocator->flush();

  if (bcAllocator->cells().empty())
  {
    std::cout << "ERROR: no cells." << std::endl;
    return false;
  }

  {
    // for the last material, construct a cell set containing only the cells of
    // this material type
    smtk::mesh::CellSet cellsForMaterial(meshResource, (bcAllocator->cells() - cellsWithMaterials));

    // construct a mesh set from these cells
    smtk::mesh::MeshSet meshForMaterial = meshResource->createMesh(cellsForMaterial);

    // set the domain on the mesh set
    meshResource->setDomainOnMeshes(meshForMaterial, smtk::mesh::Domain(currentMaterialId));
  }

  // When cells are not contiguous by domain in the file, multiple meshsets are
  // created for each domain. Condense the meshsets so that there is one meshset
  // per domain.
  condenseMeshsetsByDomain(meshResource);

  return true;
}

bool read_dm(std::istream& stream, smtk::mesh::ResourcePtr& meshResource)
{
  bool success = false;

  if (!meshResource)
  {
    return success;
  }

  smtk::mesh::BufferedCellAllocatorPtr bcAllocator =
    meshResource->interface()->bufferedCellAllocator();

  success = readPoints(stream, bcAllocator);
  if (!success)
  {
    return success;
  }

  success = readCells(stream, bcAllocator, meshResource);

  return success;
}
} // namespace
MeshIOXMS::MeshIOXMS()
{
  this->Formats.emplace_back(
    "xms 2d", std::vector<std::string>({ ".2dm" }), Format::Import | Format::Export);
  this->Formats.emplace_back(
    "xms 3d", std::vector<std::string>({ ".3dm" }), Format::Import | Format::Export);
}

smtk::mesh::ResourcePtr MeshIOXMS::importMesh(
  const std::string& filePath,
  const smtk::mesh::InterfacePtr& interface,
  const std::string& str) const
{
  smtk::mesh::ResourcePtr meshResource = smtk::mesh::Resource::create(interface);
  if (MeshIOXMS::importMesh(filePath, meshResource, str))
  {
    return meshResource;
  }

  return smtk::mesh::ResourcePtr();
}

bool MeshIOXMS::importMesh(
  const std::string& filePath,
  smtk::mesh::ResourcePtr meshResource,
  const std::string& /*unused*/) const
{
  ::boost::filesystem::path path(filePath);
  if (!::boost::filesystem::is_regular_file(path))
  {
    return false;
  }
  std::ifstream ifs(filePath.c_str(), std::ifstream::in);
  bool success = read_dm(ifs, meshResource);
  meshResource->interface()->setModifiedState(false);
  return success;
}

bool MeshIOXMS::exportMesh(
  std::ostream& stream,
  smtk::mesh::ResourcePtr meshResource,
  smtk::mesh::DimensionType dim) const
{
  return write_dm(meshResource, stream, dim);
}

bool MeshIOXMS::exportMesh(
  const std::string& filePath,
  smtk::mesh::ResourcePtr meshResource,
  smtk::mesh::DimensionType dim) const
{
  bool result = false;
  OpenFile of(filePath);
  if (of.m_canWrite)
  {
    result = this->exportMesh(of.m_stream, meshResource, dim);
    of.fileWritten(result);
  }
  return result;
}

bool MeshIOXMS::exportMesh(const std::string& filePath, smtk::mesh::ResourcePtr meshResource) const
{
  // Grab the file extension
  boost::filesystem::path fpath(filePath);
  std::string ext = fpath.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  if (ext == ".2dm")
  {
    return this->exportMesh(filePath, meshResource, smtk::mesh::Dims2);
  }
  else
  {
    return this->exportMesh(filePath, meshResource, smtk::mesh::Dims3);
  }
}

bool MeshIOXMS::exportMesh(
  std::ostream& stream,
  smtk::mesh::ResourcePtr meshResource,
  smtk::model::ResourcePtr resource,
  const std::string& modelPropertyName,
  smtk::mesh::DimensionType dim) const
{
  return write_dm(meshResource, resource, modelPropertyName, stream, dim);
}

bool MeshIOXMS::exportMesh(
  const std::string& filePath,
  smtk::mesh::ResourcePtr meshResource,
  smtk::model::ResourcePtr resource,
  const std::string& modelPropertyName,
  smtk::mesh::DimensionType dim) const
{
  bool result = false;
  OpenFile of(filePath);
  if (of.m_canWrite)
  {
    result = this->exportMesh(of.m_stream, meshResource, resource, modelPropertyName, dim);
    of.fileWritten(result);
  }
  return result;
}

bool MeshIOXMS::exportMesh(
  const std::string& filePath,
  smtk::mesh::ResourcePtr meshResource,
  smtk::model::ResourcePtr resource,
  const std::string& modelPropertyName) const
{
  // Grab the file extension
  boost::filesystem::path fpath(filePath);
  std::string ext = fpath.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  if (ext == ".2dm")
  {
    return this->exportMesh(filePath, meshResource, resource, modelPropertyName, smtk::mesh::Dims2);
  }
  else
  {
    return this->exportMesh(filePath, meshResource, resource, modelPropertyName, smtk::mesh::Dims3);
  }
}
} // namespace mesh
} // namespace io
} // namespace smtk
