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

#include "smtk/mesh/CellField.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/ForEachTypes.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/PointField.h"

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

class HistogramFieldData
{
public:
  HistogramFieldData(std::size_t nBins, double min, double max)
    : m_min(min)
    , m_max(max)
  {
    m_hist.resize(nBins, 0);
  }

  const std::vector<std::size_t>& histogram() const { return m_hist; }

protected:
  std::vector<std::size_t> m_hist;
  double m_min;
  double m_max;
};

class HistogramCellFieldData : public smtk::mesh::CellForEach, public HistogramFieldData
{
public:
  HistogramCellFieldData(std::size_t nBins, double min, double max, smtk::mesh::CellField& cf)
    : smtk::mesh::CellForEach(false)
    , HistogramFieldData(nBins, min, max)
    , m_cellField(cf)
  {
  }

  void forCell(const smtk::mesh::Handle& cellId, smtk::mesh::CellType, int)
  {
    smtk::mesh::HandleRange range;
    range.insert(cellId);
    double value = 0.;
    this->m_cellField.get(range, &value);
    std::size_t bin = static_cast<std::size_t>((value - m_min) / (m_max - m_min) * m_hist.size());
    ++m_hist[bin];
  }

protected:
  smtk::mesh::CellField& m_cellField;
};

class HistogramPointFieldData : public smtk::mesh::PointForEach, public HistogramFieldData
{
public:
  HistogramPointFieldData(std::size_t nBins, double min, double max, smtk::mesh::PointField& pf)
    : smtk::mesh::PointForEach()
    , HistogramFieldData(nBins, min, max)
    , m_pointField(pf)
  {
  }

  void forPoints(const smtk::mesh::HandleRange& pointIds, std::vector<double>&, bool&)
  {
    std::vector<double> values(pointIds.size());
    this->m_pointField.get(pointIds, &values[0]);
    for (auto& value : values)
    {
      std::size_t bin = static_cast<std::size_t>((value - m_min) / (m_max - m_min) * m_hist.size());

      ++m_hist[bin];
    }
  }

protected:
  smtk::mesh::PointField& m_pointField;
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

  // Identify available sessions
  std::cout << "Available sessions\n";
  typedef smtk::model::StringList StringList;
  StringList sessions = manager->sessionTypeNames();
  smtk::mesh::ManagerPtr meshManager = manager->meshes();
  for (StringList::iterator it = sessions.begin(); it != sessions.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  // Create a new default session
  smtk::model::SessionRef sessRef = manager->createSession("native");

  // Identify available operators
  std::cout << "Available cmb operators\n";
  StringList opnames = sessRef.session()->operatorNames();
  for (StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
  {
    std::cout << "  " << *it << "\n";
  }
  std::cout << "\n";

  // Load in the model
  create_simple_mesh_model(manager, std::string(argv[1]));

  // Convert it to a mesh
  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager, manager);

  // Create an "Interpolate Mesh" operator
  smtk::model::OperatorPtr interpolateMeshOp = sessRef.session()->op("interpolate mesh");
  if (!interpolateMeshOp)
  {
    std::cerr << "No \"interpolate mesh\" operator\n";
    return 1;
  }

  // Set the operator's data set name
  bool valueSet = interpolateMeshOp->specification()->findString("dsname")->setValue("my field");

  if (!valueSet)
  {
    std::cerr << "Failed to set data set name on operator\n";
    return 1;
  }

  // Set the operator's input mesh
  smtk::mesh::MeshSet mesh = meshManager->collectionBegin()->second->meshes();
  valueSet = interpolateMeshOp->specification()->findMesh("mesh")->setValue(mesh);

  if (!valueSet)
  {
    std::cerr << "Failed to set mesh value on operator\n";
    return 1;
  }

  // Set the operator's input power
  smtk::attribute::DoubleItemPtr power = interpolateMeshOp->specification()->findDouble("power");

  if (!power)
  {
    std::cerr << "Failed to set power value on operator\n";
    return 1;
  }

  power->setValue(2.);

  bool interpolateToPoints = false;
  if (argc > 2)
  {
    interpolateToPoints = std::atoi(argv[2]) == 1;
  }

  smtk::attribute::IntItemPtr interpMode =
    interpolateMeshOp->specification()->findInt("interpmode");

  if (!interpMode)
  {
    std::cerr << "Failed to set interpolation mode on operator\n";
    return 1;
  }

  interpMode->setValue(interpolateToPoints ? 1 : 0);

  // Set the operator's input points
  std::size_t numberOfPoints = 4;
  double pointData[4][4] = { { -1., -1., 0., 0. }, { -1., 6., 0., 25. }, { 10., -1., 0., 50. },
    { 10., 6., 0., 40. } };

  bool fromCSV = false;
  if (argc > 3)
  {
    fromCSV = std::atoi(argv[3]) == 1;
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

    smtk::attribute::FileItemPtr ptsFile = interpolateMeshOp->specification()->findFile("ptsfile");
    if (!ptsFile)
    {
      std::cerr << "No \"ptsfile\" item in specification\n";
      cleanup(write_path);
      return 1;
    }

    ptsFile->setIsEnabled(true);
    ptsFile->setValue(write_path);
  }
  else
  {
    // Set the operator's input points
    smtk::attribute::GroupItemPtr points = interpolateMeshOp->specification()->findGroup("points");

    if (!points)
    {
      std::cerr << "No \"points\" item in specification\n";
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

  // Execute "Interpolate Mesh" operator...
  smtk::model::OperatorResult interpolateMeshOpResult = interpolateMeshOp->operate();

  // ...delete the generated points file...
  if (fromCSV)
  {
    cleanup(write_path);
  }

  // ...and test the results for success.
  if (interpolateMeshOpResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
  {
    std::cerr << "\"interpolate mesh\" operator failed\n";
    return 1;
  }

  // Histogram the resulting points and compare against expected values.
  std::vector<std::size_t> histogram;
  if (interpolateToPoints)
  {
    smtk::mesh::PointField pointField =
      meshManager->collectionBegin()->second->meshes().pointField("my field");
    HistogramPointFieldData histogramPointFieldData(10, -.01, 50.01, pointField);
    smtk::mesh::for_each(mesh.points(), histogramPointFieldData);
    histogram = histogramPointFieldData.histogram();
  }
  else
  {
    smtk::mesh::CellField cellField =
      meshManager->collectionBegin()->second->meshes().cellField("my field");
    HistogramCellFieldData histogramCellFieldData(10, -.01, 50.01, cellField);
    smtk::mesh::for_each(mesh.cells(), histogramCellFieldData);
    histogram = histogramCellFieldData.histogram();
  }

  std::array<std::size_t, 10> expectedForCells = { { 0, 1, 4, 6, 9, 14, 20, 18, 9, 3 } };
  std::array<std::size_t, 10> expectedForPoints = { { 0, 1, 3, 1, 3, 4, 9, 5, 4, 2 } };
  std::array<std::size_t, 10>& expected =
    interpolateToPoints ? expectedForPoints : expectedForCells;

  std::size_t counter = 0;
  for (auto& bin : histogram)
  {
    if (bin != expected[counter++])
    {
      std::cerr << "\"interpolate mesh\" operator produced unexpected results\n";
      return 1;
    }
  }

  return 0;
}
