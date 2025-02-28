//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/UUID.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/ModelToMesh.h"
#include "smtk/io/ReadMesh.h"

#include "smtk/mesh/core/CellField.h"
#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/ForEachTypes.h"
#include "smtk/mesh/core/PointField.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/operators/GenerateHotStartData.h"

#include "smtk/model/Resource.h"
#include "smtk/model/json/jsonResource.h"
#include "smtk/model/json/jsonTessellation.h"

#include <algorithm>
#include <array>
#include <fstream>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{
std::string write_root = SMTK_SCRATCH_DIR;

void create_simple_mesh_model(smtk::model::ResourcePtr resource, std::string file_path)
{
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

void cleanup(const std::string& file_path)
{
  //first verify the file exists
  ::boost::filesystem::path path(file_path);
  if (::boost::filesystem::is_regular_file(path))
  {
    //remove the file_path if it exists.
    ::boost::filesystem::remove(path);
  }
}

class HistogramFieldData
{
public:
  HistogramFieldData(std::size_t nBins, double min, double max)
    : m_min(min)
    , m_max(max)
  {
    m_hist.resize(nBins, 0);
  }

  [[nodiscard]] const std::vector<std::size_t>& histogram() const { return m_hist; }

protected:
  std::vector<std::size_t> m_hist;
  double m_min;
  double m_max;
};

class HistogramPointFieldData
  : public smtk::mesh::PointForEach
  , public HistogramFieldData
{
public:
  HistogramPointFieldData(std::size_t nBins, double min, double max, smtk::mesh::PointField& pf)
    : HistogramFieldData(nBins, min, max)
    , m_pointField(pf)
  {
  }

  void forPoints(
    const smtk::mesh::HandleRange& pointIds,
    std::vector<double>& /*xyz*/,
    bool& /*coordinatesModified*/) override
  {
    std::vector<double> values(pointIds.size());
    m_pointField.get(pointIds, values.data());
    for (auto& value : values)
    {
      std::size_t bin = static_cast<std::size_t>((value - m_min) / (m_max - m_min) * m_hist.size());

      ++m_hist[bin];
    }
  }

protected:
  smtk::mesh::PointField& m_pointField;
};
} // namespace

// Load in a model, convert it to a mesh, and construct a dataset for that mesh
// using interpolation points. Then, histogram the values of the mesh cells
// and compare the histogram to expected values.

int main(int argc, char* argv[])
{
  if (argc == 1)
  {
    std::cout << "Must provide input file as argument" << std::endl;
    return 1;
  }

  std::ifstream file;
  file.open(argv[1]);
  if (!file.good())
  {
    std::cout << "Could not open file \"" << argv[1] << "\".\n\n";
    return 1;
  }

  // Create a model resource
  smtk::model::ResourcePtr resource = smtk::model::Resource::create();

  // Load in the model
  create_simple_mesh_model(resource, std::string(argv[1]));

  // Convert it to a mesh
  smtk::io::ModelToMesh convert;
  smtk::mesh::ResourcePtr meshResource = convert(resource);

  // Create a "Generate Hotstart Data" operator
  smtk::operation::Operation::Ptr generateHotStartDataOp =
    smtk::mesh::GenerateHotStartData::create();
  if (!generateHotStartDataOp)
  {
    std::cerr << "No \"generate hotstart data\" operator\n";
    return 1;
  }

  // Set the operator's data set type
  bool valueSet = generateHotStartDataOp->parameters()->findString("dstype")->setToDefault();

  if (!valueSet)
  {
    std::cerr << "Failed to set data set type on operator\n";
    return 1;
  }

  // Set the operator's input mesh
  smtk::mesh::MeshSet mesh = meshResource->meshes();
  valueSet = generateHotStartDataOp->parameters()->associate(smtk::mesh::Component::create(mesh));

  if (!valueSet)
  {
    std::cerr << "Failed to set mesh value on operator\n";
    return 1;
  }

  // Set the operator's input power
  smtk::attribute::DoubleItemPtr power = generateHotStartDataOp->parameters()->findDouble("power");

  if (!power)
  {
    std::cerr << "Failed to set power value on operator\n";
    return 1;
  }

  power->setValue(2.);

  // Set the operator's input points
  std::size_t numberOfPoints = 4;
  double pointData[4][3] = {
    { -1., -1., 0. }, { -1., 6., 25. }, { 10., -1., 50. }, { 10., 6., 40. }
  };

  bool fromCSV = false;
  if (argc > 2)
  {
    fromCSV = std::atoi(argv[2]) == 1;
  }

  std::string write_path;
  if (fromCSV)
  {
    write_path = std::string(write_root + "/" + smtk::common::UUID::random().toString() + ".csv");
    std::ofstream outfile(write_path.c_str());
    for (std::size_t i = 0; i < 4; i++)
    {
      outfile << pointData[i][0] << "," << pointData[i][1] << "," << pointData[i][2] << std::endl;
    }
    outfile.close();

    smtk::attribute::FileItemPtr ptsFile =
      generateHotStartDataOp->parameters()->findFile("ptsfile");
    if (!ptsFile)
    {
      std::cerr << "No \"ptsfile\" item in parameters\n";
      cleanup(write_path);
      return 1;
    }

    ptsFile->setIsEnabled(true);
    ptsFile->setValue(write_path);
  }
  else
  {
    // Set the operator's input points
    smtk::attribute::GroupItemPtr points =
      generateHotStartDataOp->parameters()->findGroup("points");

    if (!points)
    {
      std::cerr << "No \"points\" item in parameters\n";
      return 1;
    }

    points->setNumberOfGroups(numberOfPoints);
    for (std::size_t i = 0; i < numberOfPoints; i++)
    {
      for (std::size_t j = 0; j < 3; j++)
      {
        smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(points->item(i, 0))
          ->setValue(j, pointData[i][j]);
      }
    }
  }

  // Execute "generate hotstart data" operator...
  smtk::operation::Operation::Result generateHotStartDataOpResult =
    generateHotStartDataOp->operate();

  // ...delete the generated points file...
  if (fromCSV)
  {
    cleanup(write_path);
  }

  // ...and test the results for success.
  if (
    generateHotStartDataOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "\"generate hotstart data\" operator failed\n";
    return 1;
  }

  // Histogram the resulting points and compare against expected values.
  std::vector<std::size_t> histogram;
  smtk::mesh::PointField pointField = meshResource->meshes().pointField("ioh");
  HistogramPointFieldData histogramPointFieldData(10, -.01, 50.01, pointField);
  smtk::mesh::for_each(mesh.points(), histogramPointFieldData);
  histogram = histogramPointFieldData.histogram();

  std::array<std::size_t, 10> expected = { { 0, 1, 3, 1, 3, 4, 9, 5, 4, 2 } };

  std::size_t counter = 0;
  for (auto& bin : histogram)
  {
    if (bin != expected[counter++])
    {
      std::cerr << "\"generate hotstart data\" operator produced unexpected results\n";
      return 1;
    }
  }
  return 0;
}
