//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/discrete/Session.h"

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

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/ForEachTypes.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/testing/cxx/helpers.h"

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
class ValidatePoints : public smtk::mesh::PointForEach
{
public:
  ValidatePoints(std::size_t nBins, double min, double max)
    : m_min(min)
    , m_max(max)
  {
    m_hist.resize(nBins, 0);
  }

  void forPoints(const smtk::mesh::HandleRange& pointIds, std::vector<double>& xyz, bool&)
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

int main(int argc, char* argv[])
{
  if (argc < 3)
    return 1;
  smtk::model::ManagerPtr manager = smtk::model::Manager::create();

  std::cout << "Available sessions\n";
  StringList sessions = manager->sessionTypeNames();
  for (StringList::iterator it = sessions.begin(); it != sessions.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::bridge::discrete::Session::Ptr session = smtk::bridge::discrete::Session::create();
  manager->registerSession(session);

  std::cout << "Available operators\n";
  StringList opnames = session->operatorNames();
  for (StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  // read the data
  smtk::model::OperatorPtr readOp = session->op("import");
  if (!readOp)
  {
    std::cerr << "No import operator\n";
    return 1;
  }

  readOp->specification()->findFile("filename")->setValue(std::string(argv[1]));
  std::cout << "Importing " << argv[1] << "\n";
  smtk::model::OperatorResult opresult = readOp->operate();
  if (opresult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
  {
    std::cerr << "Read operator failed\n";
    return 1;
  }
  // assign model value
  smtk::model::Model model2dm = opresult->findModelEntity("created")->value();
  manager->assignDefaultNames(); // should force transcription of every entity, but doesn't yet.

  smtk::attribute::ModelEntityItemPtr meshedFaceItem = opresult->findModelEntity("mesh_created");

  if (!meshedFaceItem || !meshedFaceItem->isValid())
  {
    std::cerr << "No associated mesh!\n";
    return 1;
  }

  const smtk::model::Face& meshedFace = meshedFaceItem->value();
  auto associatedCollections = manager->meshes()->associatedCollections(meshedFace);
  smtk::mesh::CollectionPtr collection = associatedCollections[0];
  smtk::mesh::MeshSet mesh = collection->meshes();

  if (!model2dm.isValid())
  {
    std::cerr << "Reading model file failed!\n";
    return 1;
  }

  // add auxiliary geometry
  smtk::model::OperatorPtr aux_geOp = session->op("add auxiliary geometry");
  std::cout << "The url for auxiliary geometry is: " << argv[2] << std::endl;
  aux_geOp->specification()->findFile("url")->setValue(std::string(argv[2]));
  aux_geOp->associateEntity(model2dm);
  smtk::model::OperatorResult aux_geOpresult = aux_geOp->operate();
  if (aux_geOpresult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
  {
    std::cerr << "Add auxiliary geometry failed!\n";
    return 1;
  }

  smtk::model::AuxiliaryGeometry auxGo2dm = aux_geOpresult->findModelEntity("created")->value();
  std::cout << "After aux_geo op, the url inside is: " << auxGo2dm.url() << std::endl;
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

    // set input values for bathymetry filter
    elevateMesh->specification()->findModelEntity("auxiliary geometry")->setValue(auxGo2dm);

    double radius = 7.;

    elevateMesh->specification()->findDouble("radius")->setValue(radius);
    elevateMesh->specification()->findMesh("mesh")->appendValue(mesh);
    elevateMesh->specification()->findVoid("invert scalars")->setIsEnabled(false);

    smtk::model::OperatorResult bathyResult = elevateMesh->operate();
    if (bathyResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
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
    // create the undo warp mesh operator
    std::cout << "Creating undo warp mesh operator\n";
    smtk::model::OperatorPtr undoWarpMesh = session->op("undo warp mesh");
    if (!undoWarpMesh)
    {
      std::cerr << "No Undo Warp Mesh operator!\n";
      return 1;
    }

    undoWarpMesh->specification()->findMesh("mesh")->appendValue(mesh);

    smtk::model::OperatorResult result = undoWarpMesh->operate();
    if (result->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
    {
      std::cerr << "Undo warp mesh operator failed\n";
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

// This macro ensures the vtk io library is loaded into the executable
smtkComponentInitMacro(smtk_elevate_mesh_operator)
  smtkComponentInitMacro(smtk_undo_warp_mesh_operator)
