//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/Displace.h"

#include "smtk/io/LoadJSON.h"
#include "smtk/io/ModelToMesh.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/mesh/testing/cxx/helpers.h"

#include <cmath>
#include <fstream>
#include <sstream>

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

void create_simple_mesh_model(smtk::model::ManagerPtr mgr)
{
  std::string file_path(data_root);
  file_path += "/model/2d/smtk/test2D.json";

  std::ifstream file(file_path.c_str());

  std::string json((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));

  //we should load in the test2D.json file as an smtk to model
  smtk::io::LoadJSON::intoModelManager(json.c_str(), mgr);
  mgr->assignDefaultNames();

  file.close();
}

void verify_empty_elevate()
{
  smtk::mesh::ManagerPtr mgr = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr collection = mgr->makeCollection();

  smtk::mesh::MeshSet emptyMeshes = collection->meshes("bad_name");
  smtk::mesh::PointSet emptyPoints = emptyMeshes.points();
  test(emptyPoints.is_empty() == true);
  test(emptyPoints.size() == 0);

  {
    double* xyzs = NULL;
    std::size_t numPoints = 0;
    const bool result = smtk::mesh::elevate(xyzs, numPoints, emptyMeshes, 0.0);
    test((result == false));
  }

  {
    float* xyzs = NULL;
    std::size_t numPoints = 0;
    const bool result = smtk::mesh::elevate(xyzs, numPoints, emptyMeshes, 0.0);
    test((result == false));
  }

  {
    double* xyzs = NULL;
    std::size_t numPoints = 0;
    const bool result = smtk::mesh::elevate(xyzs, numPoints, emptyPoints, 0.0);
    test((result == false));
  }

  {
    float* xyzs = NULL;
    std::size_t numPoints = 0;
    const bool result = smtk::mesh::elevate(xyzs, numPoints, emptyPoints, 0.0);
    test((result == false));
  }
}

template <typename T>
void verify_elevate_pointcloud()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_mesh_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr collection = convert(meshManager, modelManager);
  test(collection->isValid(), "collection should be valid");

  smtk::mesh::MeshSet meshes = collection->meshes();
  smtk::mesh::PointSet points = meshes.points();

  std::vector<T> pointCloud, originalZs;
  points.get(pointCloud);
  originalZs.reserve(pointCloud.size() / 3);

  //Set all the pointCloud Z values to a -i, that way we can
  //confirm post elevation that everything is correct
  for (std::size_t i = 0; i < pointCloud.size(); i += 3)
  {
    originalZs.push_back(pointCloud[i + 2]);
    pointCloud[i + 2] = static_cast<T>(i) * T(-1.0f);
  }

  {
    smtk::mesh::elevate(&pointCloud[0], (pointCloud.size() / 3), meshes, 0.0);

    //verify that the elevate filter doesn't add any new points
    //to the collection
    test(collection->points().size() == points.size());
  } //verify the elevate can safely leave scope

  //confirm all the points have a z value of -i
  points.get(pointCloud);
  for (std::size_t i = 0; i < pointCloud.size(); i += 3)
  {
    const T correct_z = static_cast<T>(i) * T(-1.0f);
    test((pointCloud[i + 2] == correct_z));
  }
}

//phony structured data with 300 x 300 data points, origin at (-15,15) and a
//spacing of (.1,-.1). The actual values returned are the sum of the x and y
//coordinates. For fun, we omit the data point at every 7th x coordinate and
//every 13th y coordinate.
class TestElevationStructuredData : public smtk::mesh::ElevationStructuredData
{
public:
  TestElevationStructuredData()
  {
    m_extent[0] = m_extent[2] = -150;
    m_extent[1] = m_extent[3] = 150;
    m_origin[0] = -15.;
    m_origin[1] = 15.;
    m_spacing[0] = .1;
    m_spacing[1] = -.1;
  }
  std::pair<double, double> coords(int ix, int iy) const
  {
    return std::make_pair(m_origin[0] + ix * m_spacing[0], m_origin[1] + iy * m_spacing[1]);
  }
  double operator()(int ix, int iy) const
  {
    auto c = coords(ix, iy);
    return c.first + c.second;
  }
  bool containsIndex(int ix, int iy) const { return (ix % 7 == 1 && iy % 13 == 1 ? false : true); }
};

void verify_elevate_structuredgrid()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_mesh_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr collection = convert(meshManager, modelManager);
  test(collection->isValid(), "collection should be valid");

  smtk::mesh::MeshSet meshes = collection->meshes();
  smtk::mesh::PointSet points = meshes.points();

  {
    TestElevationStructuredData testElevationStructuredData;
    smtk::mesh::elevate(testElevationStructuredData, meshes, 2.0);

    //verify that the elevate filter doesn't add any new points
    //to the collection
    test(collection->points().size() == points.size());
  } //verify the elevate can safely leave scope

  //confirm all the points have the right z value

  //point averaging over a grid does not yield a great resolution, so our
  //epsilon is pretty high
  static const double EPSILON = 1.e-2;

  std::vector<double> pts;
  points.get(pts);
  for (std::size_t i = 0; i < pts.size(); i += 3)
  {
    const double correct_z = pts[i] + pts[i + 1];
    test(std::abs(pts[i + 2] - correct_z) < EPSILON);
  }
}

void verify_elevate_functional()
{
  smtk::mesh::ManagerPtr meshManager = smtk::mesh::Manager::create();
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  create_simple_mesh_model(modelManager);

  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr collection = convert(meshManager, modelManager);
  test(collection->isValid(), "collection should be valid");

  smtk::mesh::MeshSet meshes = collection->meshes();
  smtk::mesh::PointSet points = meshes.points();

  {
    std::function<double(double x, double y)> f = [](double x, double y) { return x - y; };
    smtk::mesh::elevate(f, meshes);

    //verify that the elevate filter doesn't add any new points
    //to the collection
    test(collection->points().size() == points.size());
  } //verify the elevate can safely leave scope

  //confirm all the points have the right z value
  static const double EPSILON = 1.e-10;

  std::vector<double> pts;
  points.get(pts);
  for (std::size_t i = 0; i < pts.size(); i += 3)
  {
    const double correct_z = pts[i] - pts[i + 1];
    test(std::abs(pts[i + 2] - correct_z) < EPSILON);
  }
}
}

int UnitTestElevate(int, char** const)
{

  std::cout << "UnitTestElevate" << std::endl;
  verify_empty_elevate();

  verify_elevate_pointcloud<double>();
  verify_elevate_pointcloud<float>();
  verify_elevate_structuredgrid();
  verify_elevate_functional();

  return 0;
}
