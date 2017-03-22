//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/polygon/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/Face.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Model.h"
#include "smtk/model/Group.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/model/SimpleModelSubphrases.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/operators/AddAuxiliaryGeometry.h"

#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"
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
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkXMLMultiBlockDataWriter.h"

#include "vtkRegressionTestImage.h"

using namespace smtk::model;

int main(int argc, char* argv[])
{
  if (argc < 2)
    return 1;
  smtk::model::ManagerPtr manager = smtk::model::Manager::create();

  std::cout << "Available sessions\n";
  StringList sessions = manager->sessionTypeNames();
  for (StringList::iterator it = sessions.begin(); it != sessions.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::bridge::polygon::Session::Ptr session = smtk::bridge::polygon::Session::create();
  manager->registerSession(session);

  std::cout << "Available cmb operators in polygon session\n";
  StringList opnames = session->operatorNames();
  for (StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  // read the data
  smtk::model::OperatorPtr readOp = session->op("import smtk model");
  if (!readOp)
    {
    std::cerr << "No import smtk model operator\n";
    return 1;
    }

  readOp->specification()->findFile("filename")->setValue(std::string(argv[1]));
  std::cout << "Importing " << argv[1] << "\n";
  smtk::model::OperatorResult ismopResult = readOp->operate();
  if (
    ismopResult->findInt("outcome")->value() !=
    smtk::model::OPERATION_SUCCEEDED)
    {
    std::cerr << "Read operator failed\n";
    return 1;
    }
  // assign model value
  smtk::model::Model simpleSMTK = ismopResult->findModelEntity("created")->value();
  manager->assignDefaultNames(); // should force transcription of every entity, but doesn't yet.

  if (!simpleSMTK.isValid())
  {
    std::cerr << "Reading model " << argv[1] << " file failed!\n";
    return 1;
  }

  // get face and edge info
  EntityRefs faces = manager->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::FACE);
  std::cout << "Faces inside model are:\n";
  for (auto face:faces)
  {
    std::cout << " " << face.name() << "\n";
  }
  EntityRefs edges = manager->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::EDGE);
  std::cout << "Edges inside model are:\n";
  for (auto edge:edges)
  {
    std::cout << " " << edge.name() << "\n";
  }

  // start delete operator
  std::cout << "Create the delete operator\n";
  smtk::model::OperatorPtr deleteOp = session->op("delete");
  if (!deleteOp)
  {
    std::cout << "No delete operator\n";
    return 1;
  }

  smtk::model::Face face1 = manager->findEntitiesByPropertyAs<Faces>("name", "face 1")[0];
  test(face1.isValid());
  smtk::model::Edge edge1 = manager->findEntitiesByPropertyAs<Edges>("name", "edge 1")[0];
  test(edge1.isValid());

  bool result(0);
  result = deleteOp->specification()->associateEntity(face1);
  test(result == 1);
  result = deleteOp->specification()->associateEntity(edge1);
  test(result == 1);
  deleteOp->specification()->findVoid("delete higher-dimensional neighbors")->setIsEnabled(true);
  deleteOp->specification()->findVoid("delete lower-dimensional neighbors")->setIsEnabled(true);

  smtk::model::OperatorResult deleteOpResult = deleteOp->operate();
  if (deleteOpResult->findInt("outcome")->value()!=
            smtk::model::OPERATION_SUCCEEDED)
  {
    std::cerr << "Delete operator failed!\n";
    return 1;
  }

  vtkNew<vtkActor> act;
  vtkNew<vtkModelMultiBlockSource> src;
  vtkNew<vtkCompositePolyDataMapper> map;
  vtkNew<vtkRenderer> ren;
  vtkNew<vtkRenderWindow> win;
  vtkNew<vtkCamera> camera;
  camera->SetPosition(20,0,20);
  camera->SetFocalPoint(10,10,-10);
  src->SetModelManager(manager);
  win->SetMultiSamples(0);
  src->AllowNormalGenerationOn();
  map->SetInputConnection(src->GetOutputPort());
  act->SetMapper(map.GetPointer());
  act->SetScale(1,1,100);

  win->AddRenderer(ren.GetPointer());
  ren->AddActor(act.GetPointer());
  ren->SetBackground(0.5, 0.5, 1);
  ren->SetActiveCamera(camera.GetPointer());
  vtkRenderWindowInteractor* iac = win->MakeRenderWindowInteractor();
  vtkInteractorStyleSwitch::SafeDownCast(iac->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
  win->SetInteractor(iac);

  win->Render();
  ren->ResetCamera();

  int reVal = vtkRegressionTestImage(win.GetPointer());
  if (reVal == vtkRegressionTester::DO_INTERACTOR)
  {
    iac->Start();
  }
  return !reVal;
}

// This macro ensures the vtk io library is loaded into the executable
smtkComponentInitMacro(smtk_apply_bathymetry_operator)
