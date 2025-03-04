//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/ImportMesh.h"
#include "smtk/io/ReadMesh.h"
#include "smtk/io/WriteMesh.h"
#include "smtk/mesh/core/PointField.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/testing/cxx/helpers.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include <cmath>

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;
std::string write_root = SMTK_SCRATCH_DIR;

smtk::mesh::ResourcePtr load_mesh()
{
  std::string file_path(data_root);
  file_path += "/mesh/2d/twoMeshes.h5m";

  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  smtk::io::importMesh(file_path, mr);
  test(mr->isValid(), "resource should be valid");

  return mr;
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

void verify_partial_pointfields()
{
  smtk::mesh::ResourcePtr mr = load_mesh();

  smtk::mesh::MeshSet mesh = mr->meshes(smtk::mesh::Dims2);
  smtk::mesh::MeshSet one = mesh.subset(0);
  smtk::mesh::MeshSet two = mesh.subset(1);

  std::vector<double> fieldValues(mesh.points().size());
  for (std::size_t i = 0; i < fieldValues.size(); i++)
  {
    fieldValues[i] = static_cast<double>(i);
  }
  mesh.createPointField("field data", 1, smtk::mesh::FieldType::Double, fieldValues.data());

  std::vector<double> fieldValuesForPointField1(one.points().size() * 2);
  for (std::size_t i = 0; i < fieldValuesForPointField1.size(); i++)
  {
    fieldValuesForPointField1[i] = static_cast<double>(i);
  }
  one.createPointField(
    "field data for set 1", 2, smtk::mesh::FieldType::Double, fieldValuesForPointField1.data());

  std::vector<double> fieldValuesForPointField2(two.points().size() * 3);
  for (std::size_t i = 0; i < fieldValuesForPointField2.size(); i++)
  {
    fieldValuesForPointField2[i] = static_cast<double>(i);
  }
  two.createPointField(
    "field data for set 2", 3, smtk::mesh::FieldType::Double, fieldValuesForPointField2.data());

  {
    std::set<smtk::mesh::PointField> pointfields = mesh.pointFields();
    for (const auto& pointfield : pointfields)
    {
      std::cout << "\"" << pointfield.name() << "\" " << pointfield.dimension() << " "
                << pointfield.size() << std::endl;
      std::vector<double> retrievedFieldValues(pointfield.size() * pointfield.dimension(), -1.);
      pointfield.get(retrievedFieldValues.data());
      for (std::size_t i = 0; i < retrievedFieldValues.size(); i++)
      {
        test(fieldValues[i] == retrievedFieldValues[i]);
      }
    }
  }

  std::vector<std::string> pointfieldnames;
  {
    std::set<smtk::mesh::PointField> pointfields = one.pointFields();
    for (const auto& pointfield : pointfields)
    {
      std::cout << "\"" << pointfield.name() << "\" " << pointfield.dimension() << std::endl;
      pointfieldnames.push_back(pointfield.name());
      std::vector<double> retrievedFieldValues(one.points().size() * pointfield.dimension(), -1.);
      pointfield.get(retrievedFieldValues.data());
      for (std::size_t i = 0; i < retrievedFieldValues.size(); i++)
      {
        test(fieldValuesForPointField1[i] == retrievedFieldValues[i]);
      }
    }

    for (auto& pointfieldname : pointfieldnames)
    {
      smtk::mesh::PointField pointfield(one, pointfieldname);
      test(one.removePointField(pointfield));
      test(pointfield.size() == 0);
      test(pointfield.dimension() == 0);
    }
  }

  {
    std::set<smtk::mesh::PointField> pointfields = one.pointFields();
    test(pointfields.empty());
  }

  {
    std::set<smtk::mesh::PointField> pointfields = two.pointFields();
    test(pointfields.size() == 2);
    for (const auto& pointfield : pointfields)
    {
      std::cout << "\"" << pointfield.name() << "\" " << pointfield.dimension() << " "
                << pointfield.size() << std::endl;
    }
  }
}

void verify_duplicate_pointfields()
{
  smtk::mesh::ResourcePtr mr = load_mesh();

  smtk::mesh::MeshSet mesh = mr->meshes(smtk::mesh::Dims2);
  smtk::mesh::MeshSet one = mesh.subset(0);
  smtk::mesh::MeshSet two = mesh.subset(1);

  // Construct a point field of dimension 1 for all of <mesh>.
  std::vector<double> fieldValues(mesh.points().size());
  for (std::size_t i = 0; i < fieldValues.size(); i++)
  {
    fieldValues[i] = static_cast<double>(i);
  }
  auto cf =
    mesh.createPointField("field data", 1, smtk::mesh::FieldType::Double, fieldValues.data());
  test(cf.isValid());

  // Try to construct a point field with the same dimension and name for a
  // subset of <mesh> (should succeed and change the values of the field for
  // the points in <one>).
  std::vector<double> fieldValuesForPointField1(one.points().size());
  for (std::size_t i = 0; i < fieldValuesForPointField1.size(); i++)
  {
    fieldValuesForPointField1[i] = static_cast<double>(i) * 2;
  }
  auto cf1 = one.createPointField(
    "field data", 1, smtk::mesh::FieldType::Double, fieldValuesForPointField1.data());
  test(cf1.isValid());

  // Verify that the field values have been updated to the new values.
  {
    std::set<smtk::mesh::PointField> pointfields = mesh.pointFields();
    for (const auto& pointfield : pointfields)
    {
      std::cout << "\"" << pointfield.name() << "\" " << pointfield.dimension() << " "
                << pointfield.size() << std::endl;
      std::vector<double> retrievedFieldValues(pointfield.size() * pointfield.dimension(), -1.);
      pointfield.get(retrievedFieldValues.data());
      for (std::size_t i = 0; i < retrievedFieldValues.size(); i++)
      {
        if (i < fieldValuesForPointField1.size())
        {
          test(fieldValuesForPointField1[i] == retrievedFieldValues[i]);
        }
        else
        {
          test(fieldValues[i] == retrievedFieldValues[i]);
        }
      }
    }
  }

  // Try to construct a point field with a different dimension and the same name
  // for a subset of <mesh> (should fail).
  std::vector<double> fieldValuesForPointField2(two.points().size() * 3);
  for (std::size_t i = 0; i < fieldValuesForPointField2.size(); i++)
  {
    fieldValuesForPointField2[i] = static_cast<double>(i);
  }
  auto cf2 = two.createPointField(
    "field data", 3, smtk::mesh::FieldType::Double, fieldValuesForPointField2.data());
  test(!cf2.isValid());

  // Try again, but change the name (should succeed).
  cf2 = two.createPointField(
    "field data 2", 3, smtk::mesh::FieldType::Double, fieldValuesForPointField2.data());
  test(cf2.isValid());
}

class SetPointData : public smtk::mesh::PointForEach
{
private:
  const std::function<double(double, double, double)>& m_dataGenerator;
  smtk::mesh::PointField& m_pointfield;

public:
  SetPointData(
    const std::function<double(double, double, double)>& dataGenerator,
    smtk::mesh::PointField& ds)
    : m_dataGenerator(dataGenerator)
    , m_pointfield(ds)
  {
  }

  void forPoints(
    const smtk::mesh::HandleRange& pointIds,
    std::vector<double>& xyz,
    bool& /*coordinatesModified*/) override
  {
    std::size_t counter = 0;
    for (auto i = smtk::mesh::rangeElementsBegin(pointIds);
         i != smtk::mesh::rangeElementsEnd(pointIds);
         ++i)
    {
      double value = m_dataGenerator(xyz[counter + 0], xyz[counter + 1], xyz[counter + 2]);
      counter += 3;
      smtk::mesh::HandleRange range;
      range.insert(*i);
      m_pointfield.set(range, &value);
    }
  }
};

class ValidatePointData : public smtk::mesh::PointForEach
{
private:
  const std::function<double(double, double, double)>& m_dataGenerator;
  smtk::mesh::PointField& m_pointfield;
  const double EPSILON = 1.e-14;

public:
  ValidatePointData(
    const std::function<double(double, double, double)>& dataGenerator,
    smtk::mesh::PointField& ds)
    : m_dataGenerator(dataGenerator)
    , m_pointfield(ds)
  {
  }

  void forPoints(
    const smtk::mesh::HandleRange& pointIds,
    std::vector<double>& xyz,
    bool& /*coordinatesModified*/) override
  {
    std::size_t counter = 0;
    for (auto i = smtk::mesh::rangeElementsBegin(pointIds);
         i != smtk::mesh::rangeElementsEnd(pointIds);
         ++i)
    {
      double expectedValue = m_dataGenerator(xyz[counter + 0], xyz[counter + 1], xyz[counter + 2]);
      counter += 3;
      smtk::mesh::HandleRange range;
      range.insert(*i);
      double value = 0.;
      m_pointfield.get(range, &value);
      test(std::abs(expectedValue - value) < EPSILON);
    }
  }
};

void verify_incremental_data_assignment()
{
  smtk::mesh::ResourcePtr mr = load_mesh();
  smtk::mesh::MeshSet mesh = mr->meshes();
  std::function<double(double, double, double)> euclideanDistance =
    [](double x, double y, double z) { return std::sqrt(x * x + y * y + z * z); };
  smtk::mesh::PointField distancePointField =
    mesh.createPointField("euclidean distance", 1, smtk::mesh::FieldType::Double);

  {
    SetPointData setPointData(euclideanDistance, distancePointField);
    smtk::mesh::for_each(mesh.points(), setPointData);
  }

  {
    ValidatePointData validatePointData(euclideanDistance, distancePointField);
    smtk::mesh::for_each(mesh.points(), validatePointData);
  }
}

void verify_pointfield_persistency()
{
  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".h5m";

  std::vector<double> fieldValues;
  {
    smtk::mesh::ResourcePtr mr = load_mesh();

    smtk::mesh::MeshSet mesh = mr->meshes(smtk::mesh::Dims2);
    smtk::mesh::MeshSet one = mesh.subset(0);

    fieldValues.resize(one.points().size());
    for (std::size_t i = 0; i < fieldValues.size(); i++)
    {
      fieldValues[i] = static_cast<double>(i);
    }
    one.createPointField("field data", 1, smtk::mesh::FieldType::Double, fieldValues.data());

    //write out the mesh.
    smtk::io::WriteMesh write;
    bool result = write(write_path, mr);
    if (!result)
    {
      cleanup(write_path);
      test(result, "failed to properly write out a valid hdf5 resource");
    }
  }

  {
    smtk::io::ReadMesh read;
    smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
    read(write_path, mr);

    //remove the file from disk
    cleanup(write_path);

    {
      smtk::mesh::MeshSet mesh = mr->meshes(smtk::mesh::Dims2);
      smtk::mesh::MeshSet two = mesh.subset(0);

      smtk::mesh::PointField pointfield = *two.pointFields().begin();
      std::vector<double> retrievedFieldValues(pointfield.size() * pointfield.dimension(), 0.);
      pointfield.get(retrievedFieldValues.data());

      test(retrievedFieldValues.size() == fieldValues.size());

      for (std::size_t i = 0; i < retrievedFieldValues.size(); i++)
      {
        test(fieldValues[i] == retrievedFieldValues[i]);
      }
    }
  }
}
} // namespace

int UnitTestPointField(int /*unused*/, char** const /*unused*/)
{
  verify_partial_pointfields();
  verify_duplicate_pointfields();
  verify_incremental_data_assignment();
  verify_pointfield_persistency();

  return 0;
}
