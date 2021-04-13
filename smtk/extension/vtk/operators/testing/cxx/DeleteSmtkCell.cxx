//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/polygon/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/attribute/VoidItem.h"
#include "smtk/model/Edge.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/Registrar.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/operators/AddAuxiliaryGeometry.h"

#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/operation/Registrar.h"
#include "smtk/operation/operators/ReadResource.h"

#include "smtk/session/polygon/Registrar.h"
#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/operators/Delete.h"
#include "smtk/session/polygon/operators/Import.h"

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
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkXMLMultiBlockDataWriter.h"

#include "vtkRegressionTestImage.h"

using namespace smtk::model;

int main(int argc, char* argv[])
{
  if (argc < 2)
    return 1;

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  {
    smtk::session::polygon::Registrar::registerTo(resourceManager);
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  {
    smtk::operation::Registrar::registerTo(operationManager);
    smtk::model::Registrar::registerTo(operationManager);
    smtk::session::polygon::Registrar::registerTo(operationManager);
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  smtk::operation::ReadResource::Ptr readOp =
    operationManager->create<smtk::operation::ReadResource>();

  readOp->parameters()->findFile("filename")->setValue(std::string(argv[1]));

  std::cout << "Importing " << argv[1] << "\n";

  smtk::operation::Operation::Result readOpResult = readOp->operate();
  test(
    readOpResult->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "Read operator failed");

  smtk::session::polygon::Resource::Ptr modelresource =
    smtk::dynamic_pointer_cast<smtk::session::polygon::Resource>(
      readOpResult->findResource("resource")->value(0));

  smtk::model::Models models =
    modelresource->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);

  if (models.empty())
    return 1;

  smtk::model::Model simpleSMTK = models[0];

  if (!simpleSMTK.isValid())
  {
    std::cerr << "Reading model " << argv[1] << " file failed!\n";
    return 1;
  }

  // get face and edge info
  EntityRefs faces = modelresource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::FACE);
  std::cout << "Faces inside model are:\n";
  for (const auto& face : faces)
  {
    std::cout << " " << face.name() << "\n";
  }
  EntityRefs edges = modelresource->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::EDGE);
  std::cout << "Edges inside model are:\n";
  for (const auto& edge : edges)
  {
    std::cout << " " << edge.name() << "\n";
  }

  // start delete operator
  std::cout << "Create the delete operator\n";
  smtk::session::polygon::Delete::Ptr deleteOp =
    operationManager->create<smtk::session::polygon::Delete>();
  if (!deleteOp)
  {
    std::cout << "No delete operator\n";
    return 1;
  }

  smtk::model::Face face1 = modelresource->findEntitiesByPropertyAs<Faces>("name", "face 1")[0];
  test(face1.isValid());
  smtk::model::Edge edge1 = modelresource->findEntitiesByPropertyAs<Edges>("name", "edge 1")[0];
  test(edge1.isValid());

  bool result(false);
  result = deleteOp->parameters()->associateEntity(face1);
  test(result == 1);
  result = deleteOp->parameters()->associateEntity(edge1);
  test(result == 1);
  deleteOp->parameters()->findVoid("delete higher-dimensional neighbors")->setIsEnabled(true);
  deleteOp->parameters()->findVoid("delete lower-dimensional neighbors")->setIsEnabled(true);

  smtk::session::polygon::Delete::Result deleteOpResult = deleteOp->operate();
  if (
    deleteOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
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
  camera->SetPosition(20, 0, 20);
  camera->SetFocalPoint(10, 10, -10);
  src->SetModelResource(modelresource);
  win->SetMultiSamples(0);
  src->AllowNormalGenerationOn();
  map->SetInputConnection(src->GetOutputPort());
  act->SetMapper(map.GetPointer());
  act->SetScale(1, 1, 100);

  win->AddRenderer(ren.GetPointer());
  ren->AddActor(act.GetPointer());
  ren->SetBackground(0.5, 0.5, 1);
  ren->SetActiveCamera(camera.GetPointer());
  vtkRenderWindowInteractor* iac = win->MakeRenderWindowInteractor();
  vtkInteractorStyleSwitch::SafeDownCast(iac->GetInteractorStyle())
    ->SetCurrentStyleToTrackballCamera();
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
