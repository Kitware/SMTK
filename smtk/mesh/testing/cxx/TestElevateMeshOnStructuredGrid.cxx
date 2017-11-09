//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/mesh/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/io/ExportMesh.h"

#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/SimpleModelSubphrases.h"
#include "smtk/model/Tessellation.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/ForEachTypes.h"
#include "smtk/mesh/core/Manager.h"
#include "smtk/mesh/testing/cxx/helpers.h"
#include "smtk/mesh/utility/Metrics.h"

#include "smtk/extension/vtk/source/PointCloudFromVTKAuxiliaryGeometry.h"
#include "smtk/extension/vtk/source/StructuredGridFromVTKAuxiliaryGeometry.h"
#include "smtk/extension/vtk/source/vtkMeshMultiBlockSource.h"

#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkCompositePolyDataMapper.h"
#include "vtkDataSetAttributes.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkXMLMultiBlockDataWriter.h"

#include <cstdlib>

namespace
{
//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

static bool pcRegistered =
  smtk::extension::vtk::mesh::PointCloudFromVTKAuxiliaryGeometry::registerClass();
static bool sgRegistered =
  smtk::extension::vtk::mesh::StructuredGridFromVTKAuxiliaryGeometry::registerClass();

class ValidatePoints : public smtk::mesh::PointForEach
{
public:
  ValidatePoints(std::size_t nBins, double min, double max)
    : m_min(min)
    , m_max(max)
  {
    m_hist.resize(nBins, 0);
  }

  void forPoints(const smtk::mesh::HandleRange& pointIds, std::vector<double>& xyz, bool&) override
  {
    typedef smtk::mesh::HandleRange::const_iterator c_it;
    int counter = 0;
    for (c_it i = pointIds.begin(); i != pointIds.end(); ++i, counter += 3)
    {
      std::size_t bin = xyz[counter + 2] < m_min ? 0 : xyz[counter + 2] >= m_max
          ? m_hist.size() - 1
          : static_cast<std::size_t>((xyz[counter + 2] - m_min) / (m_max - m_min) * m_hist.size());
      ++m_hist[bin];
    }
  }

  const std::vector<std::size_t>& histogram() const { return m_hist; }

private:
  std::vector<std::size_t> m_hist;
  double m_min;
  double m_max;
};
}

using namespace smtk::model;

int TestElevateMeshOnStructuredGrid(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  smtk::model::ManagerPtr manager = smtk::model::Manager::create();

  std::cout << "Available sessions\n";
  StringList sessions = manager->sessionTypeNames();
  for (StringList::iterator it = sessions.begin(); it != sessions.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::bridge::mesh::Session::Ptr session = smtk::bridge::mesh::Session::create();
  manager->registerSession(session);

  std::cout << "Available operators\n";
  StringList opnames = session->operatorNames();
  for (StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::model::OperatorPtr importOp = session->op("import");
  if (!importOp)
  {
    std::cerr << "No import operator\n";
    return 1;
  }

  std::string importFilePath(data_root);
  importFilePath += "/mesh/2d/testSurfaceEdgesSmall.2dm";

  importOp->specification()->findFile("filename")->setValue(importFilePath);
  importOp->specification()->findVoid("construct hierarchy")->setIsEnabled(false);

  smtk::model::OperatorResult importOpResult = importOp->operate();

  if (importOpResult->findInt("outcome")->value() != smtk::operation::Operator::OPERATION_SUCCEEDED)
  {
    std::cerr << "Import operator failed\n";
    return 1;
  }

  smtk::model::Model model = importOpResult->findModelEntity("model")->value();

  if (!model.isValid())
  {
    std::cerr << "Import operator returned an invalid model\n";
    return 1;
  }

  auto associatedCollections = manager->meshes()->associatedCollections(model);
  smtk::mesh::CollectionPtr collection = associatedCollections[0];
  smtk::mesh::MeshSet mesh = collection->meshes();

  // add auxiliary geometry
  smtk::model::OperatorPtr aux_geOp = session->op("add auxiliary geometry");
  {
    std::string file_path(data_root);
    file_path += "/image/tiff/testSurfaceEdgesSmall.tiff";
    aux_geOp->specification()->findFile("url")->setValue(file_path);
  }
  aux_geOp->associateEntity(model);
  smtk::model::OperatorResult aux_geOpresult = aux_geOp->operate();
  if (aux_geOpresult->findInt("outcome")->value() != smtk::operation::Operator::OPERATION_SUCCEEDED)
  {
    std::cerr << "Add auxiliary geometry failed!\n";
    return 1;
  }

  smtk::model::AuxiliaryGeometry auxGo2dm = aux_geOpresult->findModelEntity("created")->value();
  if (!auxGo2dm.isValid())
  {
    std::cerr << "Auxiliary geometry is not valid!\n";
    return 1;
  }

  {
    // create the elevate mesh operator
    std::cout << "Creating elevate mesh operator\n";
    smtk::model::OperatorPtr elevateMesh = session->op("elevate mesh");
    if (!elevateMesh)
    {
      std::cerr << "No Elevate Mesh operator!\n";
      return 1;
    }

    // set input values for the elevate mesh operator
    elevateMesh->specification()->findString("input data")->setToDefault();
    elevateMesh->specification()->findModelEntity("auxiliary geometry")->setValue(auxGo2dm);
    elevateMesh->specification()->findString("interpolation scheme")->setToDefault();
    elevateMesh->specification()->findDouble("radius")->setValue(7.);
    elevateMesh->specification()->findMesh("mesh")->appendValue(mesh);
    elevateMesh->specification()->findVoid("invert scalars")->setIsEnabled(false);

    smtk::model::OperatorResult bathyResult = elevateMesh->operate();
    if (bathyResult->findInt("outcome")->value() != smtk::operation::Operator::OPERATION_SUCCEEDED)
    {
      std::cerr << "Elevate mesh operator failed\n";
      return 1;
    }
  }

  // histogram elevated coordinate values
  auto extent = smtk::mesh::utility::extent(mesh);
  {
    ValidatePoints validatePoints(10, extent[4], extent[5]);
    smtk::mesh::for_each(mesh.points(), validatePoints);
    std::size_t valid[10] = { 116, 19, 11, 12, 24, 23, 29, 12, 28, 520 };

    for (std::size_t bin = 0; bin < 10; bin++)
    {
      test(validatePoints.histogram()[bin] == valid[bin]);
    }
  }

  {
    // create the undo elevate mesh operator
    std::cout << "Creating undo elevate mesh operator\n";
    smtk::model::OperatorPtr undoElevateMesh = session->op("undo elevate mesh");
    if (!undoElevateMesh)
    {
      std::cerr << "No Undo Elevate Mesh operator!\n";
      return 1;
    }

    undoElevateMesh->specification()->findMesh("mesh")->appendValue(mesh);

    smtk::model::OperatorResult result = undoElevateMesh->operate();
    if (result->findInt("outcome")->value() != smtk::operation::Operator::OPERATION_SUCCEEDED)
    {
      std::cerr << "Undo elevate mesh operator failed\n";
      return 1;
    }
  }

  // histogram unelevated coordinate values
  {
    ValidatePoints validatePoints(10, extent[4], extent[5]);
    smtk::mesh::for_each(mesh.points(), validatePoints);

    std::size_t nNonzeroBins = 0;
    for (std::size_t bin = 0; bin < 10; bin++)
    {
      if (validatePoints.histogram()[bin] != 0)
      {
        nNonzeroBins++;
      }
    }
    test(nNonzeroBins == 1);
  }

  return 0;
}

// This macro ensures the vtk io library is loaded into the executable
smtkComponentInitMacro(smtk_elevate_mesh_operator)
  smtkComponentInitMacro(smtk_undo_elevate_mesh_operator)
