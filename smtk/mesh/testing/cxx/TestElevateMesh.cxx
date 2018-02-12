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
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/bridge/discrete/Resource.h"
#include "smtk/bridge/discrete/operators/ImportOperator.h"

#include "smtk/extension/vtk/source/PointCloudFromVTKAuxiliaryGeometry.h"
#include "smtk/extension/vtk/source/StructuredGridFromVTKAuxiliaryGeometry.h"
#include "smtk/extension/vtk/source/vtkMeshMultiBlockSource.h"

#include "smtk/io/ExportMesh.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/ForEachTypes.h"
#include "smtk/mesh/core/Manager.h"
#include "smtk/mesh/operators/ElevateMesh.h"
#include "smtk/mesh/operators/UndoElevateMesh.h"
#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
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
      std::size_t bin = xyz[counter + 2] < m_min ? 0 : xyz[counter + 2] > m_max
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

int TestElevateMesh(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  smtk::operation::NewOp::Ptr importOp = smtk::bridge::discrete::ImportOperator::create();

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

  smtk::operation::NewOp::Result importOpResult = importOp->operate();
  if (importOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED))
  {
    std::cerr << "Import operator failed\n";
    return 1;
  }

  // Retrieve the resulting resource
  smtk::attribute::ResourceItemPtr resourceItem =
    std::dynamic_pointer_cast<smtk::attribute::ResourceItem>(
      importOpResult->findResource("resource"));

  // Access the generated resource
  smtk::bridge::discrete::Resource::Ptr resource =
    std::dynamic_pointer_cast<smtk::bridge::discrete::Resource>(resourceItem->value());

  // Retrieve the resulting model
  smtk::attribute::ComponentItemPtr componentItem =
    std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
      importOpResult->findComponent("model"));

  // Access the generated model
  smtk::model::Entity::Ptr model =
    std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

  smtk::model::Model model2dm = model->referenceAs<smtk::model::Model>();

  smtk::attribute::ModelEntityItemPtr meshedFaceItem =
    importOpResult->findModelEntity("mesh_created");

  if (!meshedFaceItem || !meshedFaceItem->isValid())
  {
    std::cerr << "No associated mesh!\n";
    return 1;
  }

  const smtk::model::Face& meshedFace = meshedFaceItem->value();
  auto associatedCollections = resource->meshes()->associatedCollections(meshedFace);
  smtk::mesh::CollectionPtr collection = associatedCollections[0];
  smtk::mesh::MeshSet mesh = collection->meshes();

  if (!model2dm.isValid())
  {
    std::cerr << "Reading model file failed!\n";
    return 1;
  }

  // add auxiliary geometry
  smtk::operation::NewOp::Ptr auxGeoOp = smtk::model::AddAuxiliaryGeometry::create();

  {
    std::string file_path(data_root);
    file_path += "/mesh/2d/SimpleBathy.2dm";
    auxGeoOp->parameters()->findFile("url")->setValue(file_path);
  }

  auxGeoOp->parameters()->associateEntity(model2dm);
  smtk::operation::NewOp::Result auxGeoOpResult = auxGeoOp->operate();
  if (auxGeoOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED))
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
    smtk::operation::NewOp::Ptr elevateMesh = smtk::mesh::ElevateMesh::create();
    if (!elevateMesh)
    {
      std::cerr << "No Elevate Mesh operator!\n";
      return 1;
    }

    // set input values for the elevate mesh operator
    elevateMesh->parameters()->findString("input data")->setToDefault();
    elevateMesh->parameters()->findModelEntity("auxiliary geometry")->setValue(auxGeo2dm);
    elevateMesh->parameters()->findString("interpolation scheme")->setToDefault();
    elevateMesh->parameters()->findDouble("radius")->setValue(7.);
    elevateMesh->parameters()->findMesh("mesh")->appendValue(mesh);

    smtk::operation::NewOp::Result bathyResult = elevateMesh->operate();
    if (bathyResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED))
    {
      std::cerr << "Elevate mesh operator failed\n";
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
    smtk::operation::NewOp::Ptr undoElevateMesh = smtk::mesh::UndoElevateMesh::create();
    if (!undoElevateMesh)
    {
      std::cerr << "No Undo Elevate Mesh operator!\n";
      return 1;
    }

    undoElevateMesh->parameters()->findMesh("mesh")->appendValue(mesh);

    smtk::operation::NewOp::Result result = undoElevateMesh->operate();
    if (result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED))
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
