//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/session/mesh/Resource.h"
#include "smtk/session/mesh/operators/Import.h"

#include "smtk/extension/vtk/source/PointCloudFromVTKAuxiliaryGeometry.h"
#include "smtk/extension/vtk/source/StructuredGridFromVTKAuxiliaryGeometry.h"

#include "smtk/io/ExportMesh.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/ForEachTypes.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/operators/ElevateMesh.h"
#include "smtk/mesh/operators/UndoElevateMesh.h"
#include "smtk/mesh/testing/cxx/helpers.h"
#include "smtk/mesh/utility/Metrics.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/operators/AddAuxiliaryGeometry.h"

#include "smtk/operation/Manager.h"

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

bool pcRegistered = smtk::extension::vtk::mesh::PointCloudFromVTKAuxiliaryGeometry::registerClass();
bool sgRegistered =
  smtk::extension::vtk::mesh::StructuredGridFromVTKAuxiliaryGeometry::registerClass();

class ValidatePoints : public smtk::mesh::PointForEach
{
public:
  ValidatePoints(std::size_t nBins, int coord, double min, double max)
    : m_coord(coord)
    , m_min(min)
    , m_max(max)
  {
    m_hist.resize(nBins, 0);
  }

  void forPoints(
    const smtk::mesh::HandleRange& pointIds,
    std::vector<double>& xyz,
    bool& /*coordinatesModified*/) override
  {
    int counter = 0;
    for (auto i = smtk::mesh::rangeElementsBegin(pointIds);
         i != smtk::mesh::rangeElementsEnd(pointIds);
         ++i, counter += 3)
    {
      std::size_t bin = xyz[counter + m_coord] < m_min ? 0
                                                       : xyz[counter + m_coord] >= m_max
          ? m_hist.size() - 1
          : static_cast<std::size_t>(
              (xyz[counter + m_coord] - m_min) / (m_max - m_min) * m_hist.size());
      ++m_hist[bin];
    }
  }

  const std::vector<std::size_t>& histogram() const { return m_hist; }

private:
  std::vector<std::size_t> m_hist;
  int m_coord;
  double m_min;
  double m_max;
};
} // namespace

using namespace smtk::model;

int TestElevateMeshOnStructuredGrid(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  auto importOp = smtk::session::mesh::Import::create();

  if (!importOp)
  {
    std::cerr << "No import operator\n";
    return 1;
  }

  std::string importFilePath(data_root);
  importFilePath += "/mesh/2d/testSurfaceEdgesSmall.2dm";

  importOp->parameters()->findFile("filename")->setValue(importFilePath);
  importOp->parameters()->findVoid("construct hierarchy")->setIsEnabled(false);

  smtk::operation::Operation::Result importOpResult = importOp->operate();

  if (
    importOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "Import operator failed\n";
    return 1;
  }

  // Retrieve the resulting resource
  smtk::attribute::ResourceItemPtr resourceItem =
    std::dynamic_pointer_cast<smtk::attribute::ResourceItem>(
      importOpResult->findResource("resourcesCreated"));

  // Access the generated resource
  smtk::session::mesh::Resource::Ptr resource =
    std::dynamic_pointer_cast<smtk::session::mesh::Resource>(resourceItem->value());

  // Retrieve the resulting model
  smtk::attribute::ComponentItemPtr componentItem =
    std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
      importOpResult->findComponent("model"));

  // Access the generated model
  smtk::model::Entity::Ptr modelEntity =
    std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

  smtk::model::Model model = modelEntity->referenceAs<smtk::model::Model>();

  if (!model.isValid())
  {
    std::cerr << "Import operator returned an invalid model\n";
    return 1;
  }

  smtk::mesh::ResourcePtr meshResource = resource->resource();
  smtk::mesh::MeshSet mesh = meshResource->meshes();

  // add auxiliary geometry
  auto auxGeoOp = smtk::model::AddAuxiliaryGeometry::create();

  {
    std::string file_path(data_root);
    file_path += "/image/tiff/testSurfaceEdgesSmall.tiff";
    auxGeoOp->parameters()->findFile("url")->setValue(file_path);
  }
  auxGeoOp->parameters()->associateEntity(model);

  smtk::operation::Operation::Result auxGeoOpResult = auxGeoOp->operate();

  if (
    auxGeoOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "Add auxiliary geometry failed!\n";
    return 1;
  }

  // Retrieve the resulting auxiliary geometry item
  smtk::attribute::ComponentItemPtr auxGeoItem =
    std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
      auxGeoOpResult->findComponent("created"));

  // Access the generated auxiliary geometry
  smtk::model::Entity::Ptr auxGeoEntity =
    std::dynamic_pointer_cast<smtk::model::Entity>(auxGeoItem->value());

  smtk::model::AuxiliaryGeometry auxGeo =
    auxGeoEntity->referenceAs<smtk::model::AuxiliaryGeometry>();

  if (!auxGeo.isValid())
  {
    std::cerr << "Auxiliary geometry is not valid!\n";
    return 1;
  }

  {
    // create the elevate mesh operator
    std::cout << "Creating elevate mesh operator\n";
    auto elevateMesh = smtk::mesh::ElevateMesh::create();
    if (!elevateMesh)
    {
      std::cerr << "No Elevate Mesh operator!\n";
      return 1;
    }

    // set input values for the elevate mesh operator
    elevateMesh->parameters()->associate(smtk::mesh::Component::create(mesh));
    elevateMesh->parameters()->findString("input data")->setToDefault();
    elevateMesh->parameters()->findComponent("auxiliary geometry")->setValue(auxGeo.component());
    elevateMesh->parameters()->findString("interpolation scheme")->setToDefault();
    elevateMesh->parameters()->findDouble("radius")->setValue(7.);
    elevateMesh->parameters()->findString("external point values")->setValue("set to value");
    elevateMesh->parameters()->findDouble("external point value")->setValue(-1.);

    smtk::operation::Operation::Result bathyResult = elevateMesh->operate();
    if (
      bathyResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Elevate mesh operator failed\n";
      return 1;
    }
  }

  ValidatePoints validatePointsX(10, 0, 0., 250.);
  smtk::mesh::for_each(mesh.points(), validatePointsX);

  // histogram elevated coordinate values
  {
    ValidatePoints validatePointsZ(10, 2, 190., 250.);
    smtk::mesh::for_each(mesh.points(), validatePointsZ);
    std::size_t valid[10] = { 127, 13, 8, 17, 27, 18, 24, 12, 24, 524 };

    for (std::size_t bin = 0; bin < 10; bin++)
    {
      test(validatePointsZ.histogram()[bin] == valid[bin]);
    }
  }

  {
    // create the undo elevate mesh operator
    std::cout << "Creating undo elevate mesh operator\n";
    auto undoElevateMesh = smtk::mesh::UndoElevateMesh::create();
    if (!undoElevateMesh)
    {
      std::cerr << "No Undo Elevate Mesh operator!\n";
      return 1;
    }

    undoElevateMesh->parameters()->associate(smtk::mesh::Component::create(mesh));

    smtk::operation::Operation::Result result = undoElevateMesh->operate();
    if (
      result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Undo elevate mesh operator failed\n";
      return 1;
    }
  }

  ValidatePoints validatePointsXFlat(10, 0, 0., 250.);
  smtk::mesh::for_each(mesh.points(), validatePointsXFlat);
  for (std::size_t bin = 0; bin < 10; bin++)
  {
    test(validatePointsX.histogram()[bin] == validatePointsXFlat.histogram()[bin]);
  }

  // histogram unelevated coordinate values
  {
    ValidatePoints validatePointsZ(10, 2, 190., 250.);
    smtk::mesh::for_each(mesh.points(), validatePointsZ);

    std::size_t nNonzeroBins = 0;
    for (std::size_t bin = 0; bin < 10; bin++)
    {
      if (validatePointsZ.histogram()[bin] != 0)
      {
        nNonzeroBins++;
      }
    }
    test(nNonzeroBins == 1);
  }

  return 0;
}
