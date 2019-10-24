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
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/session/mesh/Registrar.h"
#include "smtk/session/mesh/Session.h"
#include "smtk/session/mesh/operators/CreateUniformGrid.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Registrar.h"
#include "smtk/model/Resource.h"
#include "smtk/model/operators/AddAuxiliaryGeometry.h"

#include "smtk/resource/Manager.h"

#include "smtk/operation/Manager.h"

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

#include "vtkRegressionTestImage.h"

// This macro ensures the vtk io library is loaded into the executable
smtkComponentInitMacro(smtk_extension_vtk_io_mesh_MeshIOVTK)

  namespace
{
  int VisualizeModel(smtk::model::Model model, int argc, char* argv[])
  {
    vtkNew<vtkActor> act;
    vtkNew<vtkModelMultiBlockSource> src;
    vtkNew<vtkCompositePolyDataMapper> map;
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> win;
    src->SetModelResource(model.resource());
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

    int status = !vtkRegressionTestImage(win.GetPointer());
    return status;
  }
}

int main(int argc, char* argv[])
{
  if (argc == 1)
  {
    std::cout << "Usage: displayAuxiliaryGeometry <path/to/AuxiliaryGeometry>" << std::endl;
    return 1;
  }

  std::string auxiliaryGeometry;

  {
    std::ifstream file;
    file.open(argv[1]);
    if (file.good())
    {
      auxiliaryGeometry = std::string(argv[1]);
    }
  }

  if (auxiliaryGeometry.empty())
  {
    std::cout << "Could not open file \"" << argv[1] << "\".\n\n";
    return 1;
  }

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Register mesh resources to the resource manager
  {
    smtk::session::mesh::Registrar::registerTo(resourceManager);
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register model and mesh operators to the operation manager
  {
    smtk::model::Registrar::registerTo(operationManager);
    smtk::session::mesh::Registrar::registerTo(operationManager);
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  // Create a "create background domain" operator
  smtk::operation::Operation::Ptr createBackgroundDomainOp =
    operationManager->create<smtk::session::mesh::CreateUniformGrid>();

  if (!createBackgroundDomainOp)
  {
    std::cerr << "Couldn't create \"create uniform grid\" operator" << std::endl;
    return 1;
  }

  createBackgroundDomainOp->parameters()->findString("dimension")->setValue("2");
  createBackgroundDomainOp->parameters()->findDouble("size2d")->setValue(0, 2);

  createBackgroundDomainOp->parameters()->findInt("discretization2d")->setValue(0, 5);
  createBackgroundDomainOp->parameters()->findInt("discretization2d")->setValue(1, 5);

  smtk::operation::Operation::Result createBackgroundDomainOpResult =
    createBackgroundDomainOp->operate();

  if (createBackgroundDomainOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "\"create uniform grid\" operator failed\n";
    return 1;
  }

  smtk::attribute::ComponentItemPtr componentItem =
    std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
      createBackgroundDomainOpResult->findComponent("created"));

  smtk::model::Entity::Ptr model =
    std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

  {
    // Create an "add auxiliary geometry" operator
    smtk::operation::Operation::Ptr addAuxiliaryGeometryOp =
      operationManager->create<smtk::model::AddAuxiliaryGeometry>();

    if (!addAuxiliaryGeometryOp)
    {
      std::cerr << "Couldn't create \"add auxiliary geometry\" operator" << std::endl;
      return 1;
    }

    addAuxiliaryGeometryOp->parameters()->associateEntity(model);
    addAuxiliaryGeometryOp->parameters()->findFile("url")->setValue(auxiliaryGeometry);

    addAuxiliaryGeometryOp->parameters()->findDouble("rotate")->setValue(0, 270.);

    addAuxiliaryGeometryOp->parameters()->findDouble("translate")->setValue(0, 1.);
    addAuxiliaryGeometryOp->parameters()->findDouble("translate")->setValue(1, 1.);

    smtk::operation::Operation::Result addAuxiliaryGeometryOpResult =
      addAuxiliaryGeometryOp->operate();

    if (addAuxiliaryGeometryOpResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "\"add auxiliary geometry\" operator failed\n";
      return 1;
    }
  }

  return VisualizeModel(model->referenceAs<smtk::model::Model>(), argc, argv);
}
