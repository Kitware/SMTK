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

#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/operators/AddAuxiliaryGeometry.h"

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
  ValidatePoints(std::size_t nBins, double min, double max)
    : m_min(min)
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
      std::size_t bin = xyz[counter + 2] < m_min ? 0
                                                 : xyz[counter + 2] > m_max
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
} // namespace

using namespace smtk::model;

int TestElevateMesh(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  smtk::operation::Operation::Ptr importOp = smtk::session::mesh::Import::create();

  if (!importOp)
  {
    std::cerr << "No import operator\n";
    return 1;
  }

  {
    std::string file_path(data_root);
    file_path += "/mesh/2d/Simple.2dm";
    importOp->parameters()->findFile("filename")->setValue(file_path);
    std::cout << "Importing " << file_path << "\n";
  }

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
  smtk::model::Entity::Ptr model =
    std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

  smtk::model::Model model2dm = model->referenceAs<smtk::model::Model>();

  auto meshedFaceItem = importOpResult->findComponent("mesh_created");

  if (!meshedFaceItem || !meshedFaceItem->isValid())
  {
    std::cerr << "No associated mesh!\n";
    return 1;
  }

  smtk::model::Face meshedFace = meshedFaceItem->valueAs<smtk::model::Entity>();

  auto resources = std::dynamic_pointer_cast<smtk::attribute::ResourceItem>(
    importOpResult->findResource("resourcesCreated"));

  smtk::mesh::ResourcePtr meshResource = resource->resource();

  smtk::mesh::MeshSet mesh = meshResource->meshes();

  if (!model2dm.isValid())
  {
    std::cerr << "Reading model file failed!\n";
    return 1;
  }

  // add auxiliary geometry
  auto auxGeoOp = smtk::model::AddAuxiliaryGeometry::create();

  {
    std::string file_path(data_root);
    file_path += "/mesh/2d/SimpleBathy.vtu";
    auxGeoOp->parameters()->findFile("url")->setValue(file_path);
  }

  auxGeoOp->parameters()->associateEntity(model2dm);
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
  smtk::model::Entity::Ptr auxGeo =
    std::dynamic_pointer_cast<smtk::model::Entity>(auxGeoItem->value());

  smtk::model::AuxiliaryGeometry auxGeo2dm = auxGeo->referenceAs<smtk::model::AuxiliaryGeometry>();
  if (!auxGeo2dm.isValid())
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
    elevateMesh->parameters()->findComponent("auxiliary geometry")->setValue(auxGeo2dm.component());
    elevateMesh->parameters()->findString("interpolation scheme")->setToDefault();
    elevateMesh->parameters()->findDouble("radius")->setValue(7.);

    if (!elevateMesh->ableToOperate())
    {
      std::cerr << "Elevate mesh operator could not operate\n";
      return 1;
    }

    smtk::operation::Operation::Result bathyResult = elevateMesh->operate();
    if (
      bathyResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Elevate mesh operator failed\n";
      std::cerr << elevateMesh->log().convertToString() << "\n";
      return 1;
    }
  }

  // histogram elevated coordinate values
  {
    ValidatePoints validatePoints(3, .2, .35);
    smtk::mesh::for_each(mesh.points(), validatePoints);
    std::size_t valid[3] = { 1, 2, 2 };

    for (std::size_t bin = 0; bin < 3; bin++)
    {
      test(validatePoints.histogram()[bin] == valid[bin]);
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

  // histogram unelevated coordinate values
  {
    ValidatePoints validatePoints(3, .2, .35);
    smtk::mesh::for_each(mesh.points(), validatePoints);
    std::size_t valid[3] = { 5, 0, 0 };

    for (std::size_t bin = 0; bin < 3; bin++)
    {
      test(validatePoints.histogram()[bin] == valid[bin]);
    }
  }

  return 0;
}
