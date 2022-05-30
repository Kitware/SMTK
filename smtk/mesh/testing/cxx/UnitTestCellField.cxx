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

#include "smtk/mesh/core/CellField.h"
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

void verify_partial_cellfields()
{
  smtk::mesh::ResourcePtr mr = load_mesh();

  smtk::mesh::MeshSet mesh = mr->meshes(smtk::mesh::Dims2);
  smtk::mesh::MeshSet one = mesh.subset(0);
  smtk::mesh::MeshSet two = mesh.subset(1);

  std::vector<double> fieldValues(mesh.cells().size());
  for (std::size_t i = 0; i < fieldValues.size(); i++)
  {
    fieldValues[i] = static_cast<double>(i);
  }
  mesh.createCellField("field data", 1, smtk::mesh::FieldType::Double, fieldValues.data());

  std::vector<double> fieldValuesForCellField1(one.cells().size() * 2);
  for (std::size_t i = 0; i < fieldValuesForCellField1.size(); i++)
  {
    fieldValuesForCellField1[i] = static_cast<double>(i);
  }
  one.createCellField(
    "field data for set 1", 2, smtk::mesh::FieldType::Double, fieldValuesForCellField1.data());

  std::vector<double> fieldValuesForCellField2(two.cells().size() * 3);
  for (std::size_t i = 0; i < fieldValuesForCellField2.size(); i++)
  {
    fieldValuesForCellField2[i] = static_cast<double>(i);
  }
  two.createCellField(
    "field data for set 2", 3, smtk::mesh::FieldType::Double, fieldValuesForCellField2.data());

  {
    std::set<smtk::mesh::CellField> cellfields = mesh.cellFields();
    for (const auto& cellfield : cellfields)
    {
      std::cout << "\"" << cellfield.name() << "\" " << cellfield.dimension() << " "
                << cellfield.size() << std::endl;
      std::vector<double> retrievedFieldValues(cellfield.size() * cellfield.dimension(), -1.);
      cellfield.get(retrievedFieldValues.data());
      for (std::size_t i = 0; i < retrievedFieldValues.size(); i++)
      {
        test(fieldValues[i] == retrievedFieldValues[i]);
      }
    }
  }

  std::vector<std::string> cellfieldnames;
  {
    std::set<smtk::mesh::CellField> cellfields = one.cellFields();
    for (const auto& cellfield : cellfields)
    {
      std::cout << "\"" << cellfield.name() << "\" " << cellfield.dimension() << std::endl;
      cellfieldnames.push_back(cellfield.name());
      std::vector<double> retrievedFieldValues(one.cells().size() * cellfield.dimension(), -1.);
      cellfield.get(retrievedFieldValues.data());
      for (std::size_t i = 0; i < retrievedFieldValues.size(); i++)
      {
        test(fieldValuesForCellField1[i] == retrievedFieldValues[i]);
      }
    }

    for (auto& cellfieldname : cellfieldnames)
    {
      smtk::mesh::CellField cellfield(one, cellfieldname);
      test(one.removeCellField(cellfield));
      test(cellfield.size() == 0);
      test(cellfield.dimension() == 0);
    }
  }

  {
    std::set<smtk::mesh::CellField> cellfields = one.cellFields();
    test(cellfields.empty());
  }

  {
    std::set<smtk::mesh::CellField> cellfields = two.cellFields();
    test(cellfields.size() == 2);
    for (const auto& cellfield : cellfields)
    {
      std::cout << "\"" << cellfield.name() << "\" " << cellfield.dimension() << " "
                << cellfield.size() << std::endl;
    }
  }
}

void verify_duplicate_cellfields()
{
  smtk::mesh::ResourcePtr mr = load_mesh();

  smtk::mesh::MeshSet mesh = mr->meshes(smtk::mesh::Dims2);
  smtk::mesh::MeshSet one = mesh.subset(0);
  smtk::mesh::MeshSet two = mesh.subset(1);

  // Construct a cell field of dimension 1 for all of <mesh>.
  std::vector<double> fieldValues(mesh.cells().size());
  for (std::size_t i = 0; i < fieldValues.size(); i++)
  {
    fieldValues[i] = static_cast<double>(i);
  }
  auto cf =
    mesh.createCellField("field data", 1, smtk::mesh::FieldType::Double, fieldValues.data());
  test(cf.isValid());

  // Try to construct a cell field with the same dimension and name for a subset
  // of <mesh> (should succeed and change the values of the field for the cells
  // in <one>).
  std::vector<double> fieldValuesForCellField1(one.cells().size());
  for (std::size_t i = 0; i < fieldValuesForCellField1.size(); i++)
  {
    fieldValuesForCellField1[i] = static_cast<double>(i) * 2;
  }
  auto cf1 = one.createCellField(
    "field data", 1, smtk::mesh::FieldType::Double, &fieldValuesForCellField1[0]);
  test(cf1.isValid());

  // Verify that the field values have been updated to the new values.
  {
    std::set<smtk::mesh::CellField> cellfields = mesh.cellFields();
    for (const auto& cellfield : cellfields)
    {
      std::cout << "\"" << cellfield.name() << "\" " << cellfield.dimension() << " "
                << cellfield.size() << std::endl;
      std::vector<double> retrievedFieldValues(cellfield.size() * cellfield.dimension(), -1.);
      cellfield.get(retrievedFieldValues.data());
      for (std::size_t i = 0; i < retrievedFieldValues.size(); i++)
      {
        if (i < fieldValuesForCellField1.size())
        {
          test(fieldValuesForCellField1[i] == retrievedFieldValues[i]);
        }
        else
        {
          test(fieldValues[i] == retrievedFieldValues[i]);
        }
      }
    }
  }

  // Try to construct a cell field with a different dimension and the same name
  // for a subset of <mesh> (should fail).
  std::vector<double> fieldValuesForCellField2(two.cells().size() * 3);
  for (std::size_t i = 0; i < fieldValuesForCellField2.size(); i++)
  {
    fieldValuesForCellField2[i] = static_cast<double>(i);
  }
  auto cf2 = two.createCellField(
    "field data", 3, smtk::mesh::FieldType::Double, &fieldValuesForCellField2[0]);
  test(!cf2.isValid());

  // Try again, but change the name (should succeed).
  cf2 = two.createCellField(
    "field data 2", 3, smtk::mesh::FieldType::Double, &fieldValuesForCellField2[0]);
  test(cf2.isValid());
}

class SetCellData : public smtk::mesh::CellForEach
{
private:
  const std::function<double(double, double, double)>& m_dataGenerator;
  smtk::mesh::CellField& m_cellfield;

public:
  SetCellData(
    const std::function<double(double, double, double)>& dataGenerator,
    smtk::mesh::CellField& ds)
    : smtk::mesh::CellForEach(true)
    , m_dataGenerator(dataGenerator)
    , m_cellfield(ds)
  {
  }

  void forCell(const smtk::mesh::Handle& cellId, smtk::mesh::CellType /*cellType*/, int numPts)
    override
  {
    double xyz[3] = { 0., 0., 0. };
    for (int i = 0; i < numPts; i++)
    {
      xyz[0] += this->coordinates()[i * 3 + 0];
      xyz[1] += this->coordinates()[i * 3 + 1];
      xyz[2] += this->coordinates()[i * 3 + 2];
    }
    for (int i = 0; i < 3; i++)
    {
      xyz[i] /= numPts;
    }
    double value = m_dataGenerator(xyz[0], xyz[1], xyz[2]);
    smtk::mesh::HandleRange range;
    range.insert(cellId);
    m_cellfield.set(range, &value);
  }
};

class ValidateCellData : public smtk::mesh::CellForEach
{
private:
  const std::function<double(double, double, double)>& m_dataGenerator;
  smtk::mesh::CellField& m_cellfield;
  const double EPSILON = 1.e-14;

public:
  ValidateCellData(
    const std::function<double(double, double, double)>& dataGenerator,
    smtk::mesh::CellField& ds)
    : smtk::mesh::CellForEach(true)
    , m_dataGenerator(dataGenerator)
    , m_cellfield(ds)
  {
  }

  void forCell(const smtk::mesh::Handle& cellId, smtk::mesh::CellType /*cellType*/, int numPts)
    override
  {
    double xyz[3] = { 0., 0., 0. };
    for (int i = 0; i < numPts; i++)
    {
      xyz[0] += this->coordinates()[i * 3 + 0];
      xyz[1] += this->coordinates()[i * 3 + 1];
      xyz[2] += this->coordinates()[i * 3 + 2];
    }
    for (int i = 0; i < 3; i++)
    {
      xyz[i] /= numPts;
    }
    double expectedValue = m_dataGenerator(xyz[0], xyz[1], xyz[2]);
    smtk::mesh::HandleRange range;
    range.insert(cellId);
    double value = 0.;
    m_cellfield.get(range, &value);

    test(std::abs(expectedValue - value) < EPSILON);
  }
};

void verify_incremental_data_assignment()
{
  smtk::mesh::ResourcePtr mr = load_mesh();
  smtk::mesh::MeshSet mesh = mr->meshes();
  std::function<double(double, double, double)> euclideanDistance =
    [](double x, double y, double z) { return std::sqrt(x * x + y * y + z * z); };
  smtk::mesh::CellField distanceCellField =
    mesh.createCellField("euclidean distance", 1, smtk::mesh::FieldType::Double);

  {
    SetCellData setCellData(euclideanDistance, distanceCellField);
    smtk::mesh::for_each(mesh.cells(), setCellData);
  }

  {
    ValidateCellData validateCellData(euclideanDistance, distanceCellField);
    smtk::mesh::for_each(mesh.cells(), validateCellData);
  }
}

void verify_cellfield_persistency()
{
  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".h5m";

  std::vector<double> fieldValues;
  {
    smtk::mesh::ResourcePtr mr = load_mesh();

    smtk::mesh::MeshSet mesh = mr->meshes(smtk::mesh::Dims2);
    smtk::mesh::MeshSet one = mesh.subset(0);

    fieldValues.resize(one.cells().size());
    for (std::size_t i = 0; i < fieldValues.size(); i++)
    {
      fieldValues[i] = static_cast<double>(i);
    }
    one.createCellField("field data", 1, smtk::mesh::FieldType::Double, fieldValues.data());

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

      smtk::mesh::CellField cellfield = *two.cellFields().begin();
      std::vector<double> retrievedFieldValues(cellfield.size() * cellfield.dimension(), 0.);
      cellfield.get(retrievedFieldValues.data());

      test(retrievedFieldValues.size() == fieldValues.size());

      for (std::size_t i = 0; i < retrievedFieldValues.size(); i++)
      {
        test(fieldValues[i] == retrievedFieldValues[i]);
      }
    }
  }
}
} // namespace

int UnitTestCellField(int /*unused*/, char** const /*unused*/)
{
  verify_partial_cellfields();
  verify_duplicate_cellfields();
  verify_incremental_data_assignment();
  verify_cellfield_persistency();

  return 0;
}
