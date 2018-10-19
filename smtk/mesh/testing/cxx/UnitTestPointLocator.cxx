//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/core/PointLocator.h"

#include "smtk/io/ImportMesh.h"
#include "smtk/mesh/core/Collection.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

smtk::mesh::CollectionPtr load_mesh()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  smtk::mesh::CollectionPtr c = smtk::mesh::Collection::create();
  smtk::io::importMesh(file_path, c);
  test(c->isValid(), "collection should be valid");

  return c;
}

void verify_empty_locator(const smtk::mesh::CollectionPtr& c)
{
  smtk::mesh::PointSet emptyPoints = c->meshes("bad_name").points();
  test(emptyPoints.is_empty() == true);
  test(emptyPoints.size() == 0);

  smtk::mesh::PointLocator locator1(emptyPoints);

  double* xyzs = NULL;
  std::size_t numPoints = 0;
  smtk::mesh::PointLocator locator2(c, numPoints, xyzs);
}

void verify_raw_ptr_constructors(const smtk::mesh::CollectionPtr& c)
{
  const std::size_t initialNumPoints = c->points().size();

  double d_xyzs[6] = { 400.0, 40.0, 0.02, 100.0, 150.0, 0.0 };
  float f_xyzs[6] = { -400.0f, 400.0f, 0.0f, 100.0f, -150.0f, 0.0f };
  std::size_t numPoints = 2;

  { //test raw double pointer
    smtk::mesh::PointLocator locator2(c, numPoints, d_xyzs);
    test((c->points().size() == initialNumPoints + numPoints));
  }
  test((c->points().size() == initialNumPoints));
  { //test raw float pointer
    smtk::mesh::PointLocator locator2(c, numPoints, f_xyzs);
    test((c->points().size() == initialNumPoints + numPoints));
  }
  test((c->points().size() == initialNumPoints));
}

class FindsSelf : public smtk::mesh::PointForEach
{
  smtk::mesh::PointLocator m_locator;

public:
  FindsSelf(const smtk::mesh::PointLocator& pl)
    : m_locator(pl)
  {
  }

  void forPoints(const smtk::mesh::HandleRange& pointIds, std::vector<double>& xyz,
    bool& coordinatesModified) override
  {
    //verify the coordinates and the number of points match
    test((xyz.size() == (pointIds.size() * 3)));
    coordinatesModified = false; //we are not modifying the coords

    std::size_t index = 0;
    smtk::mesh::PointLocator::LocatorResults results;
    results.want_Coordinates = true;
    for (auto i = smtk::mesh::rangeElementsBegin(pointIds);
         i != smtk::mesh::rangeElementsEnd(pointIds); ++i)
    {
      m_locator.find(xyz[3 * index], xyz[3 * index + 1], xyz[3 * index + 2], 0.0, results);

      test((results.x_s.size() == results.pointIds.size()));
      test((results.y_s.size() == results.pointIds.size()));
      test((results.z_s.size() == results.pointIds.size()));
      test((results.sqDistances.size() == 0)); //since we didn't ask for them

      //should only return a single point as inside a radius of 0.0. So verify
      //the Id and coordinates are the same
      test((results.pointIds.size() == 1));
      test((results.pointIds[0] == index));
      test((xyz[3 * index] == results.x_s[0]));
      test((xyz[3 * index + 1] == results.y_s[0]));
      test((xyz[3 * index + 2] == results.z_s[0]));
    }
  }
};

void verify_points_find_themselves(const smtk::mesh::CollectionPtr& c)
{
  //construct a point locator for all points in the mesh
  smtk::mesh::PointLocator locator(c->points());

  //now verify that each point can locate it self with the locator
  FindsSelf functor(locator);
  smtk::mesh::for_each(c->points(), functor);
}
}

int UnitTestPointLocator(int, char** const)
{
  smtk::mesh::CollectionPtr c = load_mesh();

  verify_empty_locator(c);
  verify_raw_ptr_constructors(c);

  verify_points_find_themselves(c);

  return 0;
}
