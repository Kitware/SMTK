//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/PythonAutoInit.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/openfoam/Session.h"

#include "smtk/io/ExportMesh.h"
#include "smtk/io/SaveJSON.h"

#include "smtk/mesh/core/Manager.h"

#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/SimpleModelSubphrases.h"
#include "smtk/model/Tessellation.h"

#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkCompositePolyDataMapper.h"
#include "vtkDataSetAttributes.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkNew.h"
#include "vtkPlane.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkXMLMultiBlockDataWriter.h"

#include <fstream>

namespace
{
//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

void PrintFileToScreen(const std::string& filename)
{
  std::cout << "reading " << filename << std::endl;

  std::ifstream file;
  file.open(filename);

  //Fail check
  if (file.fail())
  {
    std::cout << "Could not read generated file." << std::endl;
  }

  while (file.good())
  {
    std::cout << (char)file.get();
  }
  std::cout << std::endl;
}

void UniqueEntities(const smtk::model::EntityRef& root, std::set<smtk::model::EntityRef>& unique)
{
  smtk::model::EntityRefArray children = (root.isModel()
      ? root.as<smtk::model::Model>().cellsAs<smtk::model::EntityRefArray>()
      : (root.isCellEntity()
            ? root.as<smtk::model::CellEntity>().boundingCellsAs<smtk::model::EntityRefArray>()
            : (root.isGroup() ? root.as<smtk::model::Group>().members<smtk::model::EntityRefArray>()
                              : smtk::model::EntityRefArray())));

  for (smtk::model::EntityRefArray::const_iterator it = children.begin(); it != children.end();
       ++it)
  {
    if (unique.find(*it) == unique.end())
    {
      unique.insert(*it);
      UniqueEntities(*it, unique);
    }
  }
}

void ParseModelTopology(smtk::model::Model& model, std::size_t* count)
{
  std::set<smtk::model::EntityRef> unique;
  UniqueEntities(model, unique);

  for (auto&& entity : unique)
  {
    if (entity.dimension() >= 0 && entity.dimension() <= 3)
    {
      count[entity.dimension()]++;
      float r = static_cast<float>(entity.dimension()) / 3;
      float b = static_cast<float>(1. - r);
      const_cast<smtk::model::EntityRef&>(entity).setColor(
        (r < 1. ? r : 1.), 0., (b < 1. ? b : 1.), 1.);
    }
  }
}

void VisualizeModel(smtk::model::Model& model)
{
  vtkNew<vtkActor> act;
  vtkNew<vtkModelMultiBlockSource> src;
  vtkNew<vtkCompositePolyDataMapper> map;
  vtkNew<vtkRenderer> ren;
  vtkNew<vtkRenderWindow> win;
  src->SetModelManager(model.manager());
  src->SetModelEntityID(model.entity().toString().c_str());
  src->SetDefaultColor(1., 1., 0., 1.);
  map->SetInputConnection(src->GetOutputPort());
  act->SetMapper(map.GetPointer());
  act->GetProperty()->SetPointSize(5);
  act->GetProperty()->SetLineWidth(2);

  vtkNew<vtkCamera> camera;
  camera->SetPosition(-1., -1., -2.);
  camera->SetFocalPoint(0, 0, 0);

  ren->SetActiveCamera(camera.GetPointer());

  win->AddRenderer(ren.GetPointer());
  ren->AddActor(act.GetPointer());

  vtkRenderWindowInteractor* iac = win->MakeRenderWindowInteractor();
  vtkInteractorStyleSwitch::SafeDownCast(iac->GetInteractorStyle())
    ->SetCurrentStyleToTrackballCamera();
  win->SetInteractor(iac);

  win->Render();
  ren->ResetCamera();

  iac->Start();
}
}

int CreateWindTunnelOp(int argc, char* argv[])
{
  smtk::model::ManagerPtr manager = smtk::model::Manager::create();

  std::cout << "Available sessions\n";
  smtk::model::StringList sessions = manager->sessionTypeNames();
  for (smtk::model::StringList::iterator it = sessions.begin(); it != sessions.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::bridge::openfoam::Session::Ptr session = smtk::bridge::openfoam::Session::create();
  manager->registerSession(session);

  std::cout << "Available cmb operators\n";
  smtk::model::StringList opnames = session->operatorNames();
  for (smtk::model::StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  {
    smtk::model::OperatorPtr setMainControlsOp = session->op("set main controls");
    if (!setMainControlsOp)
    {
      std::cerr << "No set main controls operator\n";
      return 1;
    }

    smtk::model::OperatorResult setMainControlsOpResult = setMainControlsOp->operate();
    if (setMainControlsOpResult->findInt("outcome")->value() !=
      smtk::operation::Operator::OPERATION_SUCCEEDED)
    {
      std::cerr << "set main controls operator failed\n";
      return 1;
    }

    // PrintFileToScreen(session->workingDirectory() + "/system/controlDict");
  }

  smtk::model::Model windTunnel;
  {
    smtk::model::OperatorPtr createWindTunnelOp = session->op("create wind tunnel");
    if (!createWindTunnelOp)
    {
      std::cerr << "No create wind tunnel operator\n";
      return 1;
    }

    std::array<double, 6> boundingBox = { -5, 15, -4, 4, 0, 8 };

    createWindTunnelOp->specification()->findDouble("x dimensions")->setValue(0, boundingBox[0]);
    createWindTunnelOp->specification()->findDouble("x dimensions")->setValue(1, boundingBox[1]);
    createWindTunnelOp->specification()->findDouble("y dimensions")->setValue(0, boundingBox[2]);
    createWindTunnelOp->specification()->findDouble("y dimensions")->setValue(1, boundingBox[3]);
    createWindTunnelOp->specification()->findDouble("z dimensions")->setValue(0, boundingBox[4]);
    createWindTunnelOp->specification()->findDouble("z dimensions")->setValue(1, boundingBox[5]);

    std::array<int, 3> numberOfCells = { 20, 8, 8 };

    createWindTunnelOp->specification()
      ->findInt("number of cells")
      ->setValues(numberOfCells.begin(), numberOfCells.end());

    // 0: -X to +X
    int windDirectionIdx = 0;

    createWindTunnelOp->specification()
      ->findString("wind direction")
      ->setDiscreteIndex(windDirectionIdx);

    smtk::model::OperatorResult createWindTunnelOpResult = createWindTunnelOp->operate();
    if (createWindTunnelOpResult->findInt("outcome")->value() !=
      smtk::operation::Operator::OPERATION_SUCCEEDED)
    {
      std::cerr << "create wind tunnel operator failed\n";
      return 1;
    }

    // PrintFileToScreen(session->workingDirectory() + "/system/blockMeshDict");

    windTunnel = createWindTunnelOpResult->findModelEntity("created")->value();
    if (!windTunnel.isValid())
    {
      std::cerr << "create wind tunnel operator constructed an invalid model\n";
      return 1;
    }

    std::size_t count[4] = { 0, 0, 0, 0 };
    ParseModelTopology(windTunnel, count);

    std::cout << "Wind tunnel:" << std::endl;
    std::cout << count[3] << " volumes" << std::endl;
    std::cout << count[2] << " faces" << std::endl;
    std::cout << count[1] << " edges" << std::endl;
    std::cout << count[0] << " vertex groups" << std::endl;

    // VisualizeModel(model);
  }

  smtk::model::AuxiliaryGeometry obstacle;
  {
    // add auxiliary geometry
    smtk::model::OperatorPtr aux_geOp = session->op("add auxiliary geometry");
    {
      std::string file_path(data_root);
      file_path += "/model/3d/obj/motorBike.obj";
      aux_geOp->specification()->findFile("url")->setValue(file_path);
    }
    aux_geOp->associateEntity(windTunnel);
    smtk::model::OperatorResult aux_geOpresult = aux_geOp->operate();
    if (aux_geOpresult->findInt("outcome")->value() !=
      smtk::operation::Operator::OPERATION_SUCCEEDED)
    {
      std::cerr << "Add auxiliary geometry failed!\n";
      return 1;
    }

    obstacle = aux_geOpresult->findModelEntity("created")->value();
    if (!obstacle.isValid())
    {
      std::cerr << "Auxiliary geometry is not valid!\n";
      return 1;
    }
  }

  {
    smtk::model::OperatorPtr addObstacleOp = session->op("add obstacle");
    if (!addObstacleOp)
    {
      std::cerr << "No add obstacle operator\n";
      return 1;
    }

    addObstacleOp->specification()->findModelEntity("wind tunnel")->setValue(windTunnel);

    addObstacleOp->specification()->findModelEntity("obstacle")->setValue(obstacle);

    std::array<double, 6> boundingBox = { -1., 8., -.7, .7, 0., 2.5 };

    addObstacleOp->specification()->findDouble("x dimensions")->setValue(0, boundingBox[0]);
    addObstacleOp->specification()->findDouble("x dimensions")->setValue(1, boundingBox[1]);
    addObstacleOp->specification()->findDouble("y dimensions")->setValue(0, boundingBox[2]);
    addObstacleOp->specification()->findDouble("y dimensions")->setValue(1, boundingBox[3]);
    addObstacleOp->specification()->findDouble("z dimensions")->setValue(0, boundingBox[4]);
    addObstacleOp->specification()->findDouble("z dimensions")->setValue(1, boundingBox[5]);

    smtk::model::OperatorResult addObstacleOpResult = addObstacleOp->operate();
    if (addObstacleOpResult->findInt("outcome")->value() !=
      smtk::operation::Operator::OPERATION_SUCCEEDED)
    {
      std::cerr << "add obstacle operator failed\n";
      return 1;
    }

    // PrintFileToScreen(session->workingDirectory() + "/system/snappyHexMeshDict");

    windTunnel = addObstacleOpResult->findModelEntity("created")->value();
    if (!windTunnel.isValid())
    {
      std::cerr << "add obstacle operator constructed an invalid model\n";
      return 1;
    }

    std::size_t count[4] = { 0, 0, 0, 0 };
    ParseModelTopology(windTunnel, count);

    std::cout << "Wind tunnel with obstacle:" << std::endl;
    std::cout << count[3] << " volumes" << std::endl;
    std::cout << count[2] << " faces" << std::endl;
    std::cout << count[1] << " edges" << std::endl;
    std::cout << count[0] << " vertex groups" << std::endl;

    // VisualizeModel(model);
  }

  // Clean up the directory for this session.
  if (session->workingDirectoryExists())
  {
    std::cout << "removing working directory " << session->workingDirectory() << std::endl;
    session->removeWorkingDirectory();
  }

  return 0;
}

smtkComponentInitMacro(smtk_extension_vtk_io_mesh_MeshIOVTK)
  smtkPythonInitMacro(add_obstacle, smtk.bridge.openfoam.add_obstacle, true);
smtkPythonInitMacro(set_main_controls, smtk.bridge.openfoam.set_main_controls, true);
smtkPythonInitMacro(set_working_directory, smtk.bridge.openfoam.set_working_directory, true);
smtkPythonInitMacro(create_wind_tunnel, smtk.bridge.openfoam.create_wind_tunnel, true);
