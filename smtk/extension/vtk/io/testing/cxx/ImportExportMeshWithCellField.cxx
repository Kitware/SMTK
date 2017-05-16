//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/extension/vtk/io/ExportVTKData.h"
#include "smtk/extension/vtk/io/ImportVTKData.h"

#include "smtk/io/ImportMesh.h"

#include "smtk/model/DefaultSession.h"

#include "smtk/mesh/CellField.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/ForEachTypes.h"
#include "smtk/mesh/Manager.h"

#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"

#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLPolyDataWriter.h"

#include <array>
#include <fstream>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{
//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;
std::string write_root = SMTK_SCRATCH_DIR;

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

smtk::mesh::CollectionPtr load_mesh(smtk::mesh::ManagerPtr mngr)
{
  std::string file_path(data_root);
  file_path += "/mesh/2d/twoMeshes.h5m";

  smtk::mesh::CollectionPtr c = smtk::io::importMesh(file_path, mngr);

  return c;
}

class SetCellField : public smtk::mesh::CellForEach
{
private:
  const std::function<double(double, double, double)>& m_dataGenerator;
  smtk::mesh::CellField& m_cellField;

public:
  SetCellField(
    const std::function<double(double, double, double)>& dataGenerator, smtk::mesh::CellField& cf)
    : smtk::mesh::CellForEach(true)
    , m_dataGenerator(dataGenerator)
    , m_cellField(cf)
  {
  }

  void forCell(const smtk::mesh::Handle& cellId, smtk::mesh::CellType, int numPts)
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
    double value = this->m_dataGenerator(xyz[0], xyz[1], xyz[2]);
    smtk::mesh::HandleRange range;
    range.insert(cellId);
    this->m_cellField.set(range, &value);
  }
};

class ValidateCellField : public smtk::mesh::CellForEach
{
private:
  const std::function<double(double, double, double)>& m_dataGenerator;
  smtk::mesh::CellField& m_cellField;
  const double EPSILON = 1.e-14;

public:
  ValidateCellField(
    const std::function<double(double, double, double)>& dataGenerator, smtk::mesh::CellField& cf)
    : smtk::mesh::CellForEach(true)
    , m_dataGenerator(dataGenerator)
    , m_cellField(cf)
  {
  }

  void forCell(const smtk::mesh::Handle& cellId, smtk::mesh::CellType, int numPts)
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
    double expectedValue = this->m_dataGenerator(xyz[0], xyz[1], xyz[2]);
    smtk::mesh::HandleRange range;
    range.insert(cellId);
    double value = 0.;
    this->m_cellField.get(range, &value);

    test(std::abs(expectedValue - value) < EPSILON);
  }
};
}

int ImportExportMeshWithCellField(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".vtp";

  std::function<double(double, double, double)> euclideanDistance = [](
    double x, double y, double z) { return std::sqrt(x * x + y * y + z * z); };

  {
    smtk::mesh::ManagerPtr mngr = smtk::mesh::Manager::create();
    smtk::mesh::CollectionPtr c = load_mesh(mngr);
    smtk::mesh::MeshSet mesh = c->meshes();
    smtk::mesh::CellField distanceCellField = mesh.createCellField("euclidean distance", 1);

    SetCellField setCellField(euclideanDistance, distanceCellField);
    smtk::mesh::for_each(mesh.cells(), setCellField);

    smtk::extension::vtk::io::ExportVTKData exprt;
    vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
    exprt(mesh, pd);

    vtkNew<vtkXMLPolyDataWriter> writer;
    writer->SetFileName(write_path.c_str());
    writer->SetInputData(pd);
    writer->Write();
  }

  {
    vtkNew<vtkXMLPolyDataReader> reader;
    reader->SetFileName(write_path.c_str());
    reader->Update();

    cleanup(write_path);

    smtk::extension::vtk::io::ImportVTKData imprt;
    smtk::mesh::ManagerPtr mngr = smtk::mesh::Manager::create();
    smtk::mesh::CollectionPtr c = imprt(reader->GetOutput(), mngr);
    smtk::mesh::MeshSet mesh = c->meshes();

    smtk::mesh::CellField distanceCellField = mesh.cellField("euclidean distance");

    ValidateCellField validateCellField(euclideanDistance, distanceCellField);
    smtk::mesh::for_each(mesh.cells(), validateCellField);
  }

  return 0;
}
