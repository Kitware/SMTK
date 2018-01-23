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

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/LoadJSON.h"
#include "smtk/io/ModelToMesh.h"
#include "smtk/io/ReadMesh.h"

#include "smtk/model/DefaultSession.h"

#include "smtk/mesh/core/CellField.h"
#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/ForEachTypes.h"
#include "smtk/mesh/core/Manager.h"
#include "smtk/mesh/operators/ElevateMesh.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"

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

void create_simple_mesh_model(smtk::model::ManagerPtr mgr, std::string file_path)
{
  std::ifstream file(file_path.c_str());

  std::string json((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));

  smtk::io::LoadJSON::intoModelManager(json.c_str(), mgr);
  mgr->assignDefaultNames();

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

class HistogramZCoordinates : public smtk::mesh::PointForEach
{
public:
  HistogramZCoordinates(std::size_t nBins, double min, double max)
    : m_min(min)
    , m_max(max)
  {
    m_hist.resize(nBins, 0);
  }

  void forPoints(const smtk::mesh::HandleRange& pointIds, std::vector<double>& xyz, bool&) override
  {
    std::size_t counter = 0;
    typedef smtk::mesh::HandleRange::const_iterator c_it;
    for (c_it i = pointIds.begin(); i != pointIds.end(); ++i, counter += 3)
    {
      std::size_t bin =
        static_cast<std::size_t>((xyz[counter + 2] - m_min) / (m_max - m_min) * m_hist.size());
      ++m_hist[bin];
    }
  }

  const std::vector<std::size_t>& histogram() const { return m_hist; }

protected:
  std::vector<std::size_t> m_hist;
  double m_min;
  double m_max;
};
}

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

  // Create a model manager
  smtk::model::ManagerPtr manager = smtk::model::Manager::create();

  // Access the mesh manager
  smtk::mesh::ManagerPtr meshManager = manager->meshes();

  // Load in the model
  create_simple_mesh_model(manager, std::string(argv[1]));

  // Convert it to a mesh
  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager, manager);

  // Create an "Elevate Mesh" operator
  smtk::operation::NewOp::Ptr elevateMeshOp = smtk::mesh::ElevateMesh::create();
  if (!elevateMeshOp)
  {
    std::cerr << "No \"elevate mesh\" operator\n";
    return 1;
  }

  // Set the operator's input mesh
  smtk::mesh::MeshSet mesh = meshManager->collectionBegin()->second->meshes();
  bool valueSet = elevateMeshOp->parameters()->findMesh("mesh")->appendValue(mesh);

  if (!valueSet)
  {
    std::cerr << "Failed to set mesh value on operator\n";
    return 1;
  }

  elevateMeshOp->parameters()
    ->findString("interpolation scheme")
    ->setValue("inverse distance weighting");

  // Set the operator's input power
  smtk::attribute::DoubleItemPtr power = elevateMeshOp->parameters()->findDouble("power");

  if (!power)
  {
    std::cerr << "Failed to set power value on operator\n";
    return 1;
  }

  power->setValue(2.);

  // Set the operator's input points
  std::size_t numberOfPoints = 4;
  double pointData[4][4] = { { -1., -1., 0., 0. }, { -1., 6., 0., 25. }, { 10., -1., 0., 50. },
    { 10., 6., 0., 40. } };

  bool fromCSV = false;
  if (argc > 2)
  {
    fromCSV = std::atoi(argv[2]) == 1;
  }

  std::string write_path = "";
  if (fromCSV)
  {
    write_path = std::string(write_root + "/" + smtk::common::UUID::random().toString() + ".csv");
    std::ofstream outfile(write_path.c_str());
    for (std::size_t i = 0; i < 4; i++)
    {
      outfile << pointData[i][0] << "," << pointData[i][1] << "," << pointData[i][2] << ","
              << pointData[i][3] << std::endl;
    }
    outfile.close();

    elevateMeshOp->parameters()->findString("input data")->setValue("ptsfile");
    smtk::attribute::FileItemPtr ptsFile = elevateMeshOp->parameters()->findFile("ptsfile");
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
    elevateMeshOp->parameters()->findString("input data")->setValue("points");

    // Set the operator's input points
    smtk::attribute::GroupItemPtr points = elevateMeshOp->parameters()->findGroup("points");

    if (!points)
    {
      std::cerr << "No \"points\" item in parameters\n";
      return 1;
    }

    points->setNumberOfGroups(numberOfPoints);
    for (std::size_t i = 0; i < numberOfPoints; i++)
    {
      for (std::size_t j = 0; j < 4; j++)
      {
        smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(points->item(i, 0))
          ->setValue(j, pointData[i][j]);
      }
    }
  }

  // Execute "Elevate Mesh" operator...
  smtk::operation::NewOp::Result elevateMeshOpResult = elevateMeshOp->operate();

  // ...delete the generated points file...
  if (fromCSV)
  {
    cleanup(write_path);
  }

  // ...and test the results for success.
  if (elevateMeshOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED))
  {
    std::cerr << "\"elevate mesh\" operator failed\n";
    return 1;
  }

  // Histogram the resulting points and compare against expected values.
  std::vector<std::size_t> histogram;
  HistogramZCoordinates histogramZCoordinates(10, -.01, 50.01);
  smtk::mesh::for_each(mesh.points(), histogramZCoordinates);
  histogram = histogramZCoordinates.histogram();

  std::array<std::size_t, 10> expected = { { 0, 1, 3, 1, 3, 4, 9, 5, 4, 2 } };

  std::size_t counter = 0;
  for (auto& bin : histogram)
  {
    if (bin != expected[counter++])
    {
      std::cerr << "\"elevate mesh\" operator produced unexpected results\n";
      return 1;
    }
  }

  return 0;
}
