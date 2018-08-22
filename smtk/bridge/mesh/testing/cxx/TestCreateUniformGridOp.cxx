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

#include "smtk/bridge/mesh/Registrar.h"
#include "smtk/bridge/mesh/Session.h"
#include "smtk/bridge/mesh/operators/CreateUniformGrid.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Resource.h"

#include "smtk/resource/Manager.h"

#include "smtk/operation/Manager.h"

#ifdef SMTK_ENABLE_VTK_SUPPORT
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

// This macro ensures the vtk io library is loaded into the executable
smtkComponentInitMacro(smtk_extension_vtk_io_mesh_MeshIOVTK)
#endif

  namespace
{
  void UniqueEntities(const smtk::model::EntityRef& root, std::set<smtk::model::EntityRef>& unique)
  {
    smtk::model::EntityRefArray children = (root.isModel()
        ? root.as<smtk::model::Model>().cellsAs<smtk::model::EntityRefArray>()
        : (root.isCellEntity()
              ? root.as<smtk::model::CellEntity>().boundingCellsAs<smtk::model::EntityRefArray>()
              : (root.isGroup()
                    ? root.as<smtk::model::Group>().members<smtk::model::EntityRefArray>()
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

  void ParseModelTopology(smtk::model::Model model, std::size_t * count)
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

  void VisualizeModel(smtk::model::Model model)
  {
#ifdef SMTK_ENABLE_VTK_SUPPORT
    vtkNew<vtkActor> act;
    vtkNew<vtkModelMultiBlockSource> src;
    vtkNew<vtkCompositePolyDataMapper> map;
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> win;
    src->SetModelResource(model.resource());
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
#else
    (void)model;
#endif
  }

  int Test2DGrid()
  {
    // Create a resource manager
    smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

    // Register mesh resources to the resource manager
    {
      smtk::bridge::mesh::Registrar::registerTo(resourceManager);
    }

    // Create an operation manager
    smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

    // Register mesh operators to the operation manager
    {
      smtk::bridge::mesh::Registrar::registerTo(operationManager);
    }

    // Register the resource manager to the operation manager (newly created
    // resources will be automatically registered to the resource manager).
    operationManager->registerResourceManager(resourceManager);

    // Create a "create background domain" operator
    smtk::operation::Operation::Ptr createBackgroundDomainOp =
      operationManager->create<smtk::bridge::mesh::CreateUniformGrid>();

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

    std::size_t count[4] = { 0, 0, 0, 0 };
    ParseModelTopology(model->referenceAs<smtk::model::Model>(), count);

    std::cout << count[3] << " volumes" << std::endl;
    test(count[3] == 0, "There should zero volumes");
    std::cout << count[2] << " faces" << std::endl;
    test(count[2] == 1, "There should be one face");
    std::cout << count[1] << " edges" << std::endl;
    test(count[1] == 4, "There should be four edges");
    std::cout << count[0] << " vertex groups" << std::endl;
    test(count[0] == 0, "There should be zero vertex groups");

    bool debug = false;
    if (debug)
    {
      VisualizeModel(model->referenceAs<smtk::model::Model>());
    }

    return 0;
  }

  int Test3DGrid()
  {
    // Create a resource manager
    smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

    // Register mesh resources to the resource manager
    {
      smtk::bridge::mesh::Registrar::registerTo(resourceManager);
    }

    // Create an operation manager
    smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

    // Register mesh operators to the operation manager
    {
      smtk::bridge::mesh::Registrar::registerTo(operationManager);
    }

    // Register the resource manager to the operation manager (newly created
    // resources will be automatically registered to the resource manager).
    operationManager->registerResourceManager(resourceManager);

    // Create a "create background domain" operator
    smtk::operation::Operation::Ptr createBackgroundDomainOp =
      operationManager->create<smtk::bridge::mesh::CreateUniformGrid>();

    if (!createBackgroundDomainOp)
    {
      std::cerr << "Couldn't create \"create uniform grid\" operator" << std::endl;
      return 1;
    }

    createBackgroundDomainOp->parameters()->findString("dimension")->setValue("3");
    createBackgroundDomainOp->parameters()->findDouble("size3d")->setValue(0, 2);

    createBackgroundDomainOp->parameters()->findInt("discretization3d")->setValue(0, 5);
    createBackgroundDomainOp->parameters()->findInt("discretization3d")->setValue(1, 5);
    createBackgroundDomainOp->parameters()->findInt("discretization3d")->setValue(2, 5);

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

    std::size_t count[4] = { 0, 0, 0, 0 };
    ParseModelTopology(model->referenceAs<smtk::model::Model>(), count);

    std::cout << count[3] << " volumes" << std::endl;
    test(count[3] == 1, "There should one volume");
    std::cout << count[2] << " faces" << std::endl;
    test(count[2] == 6, "There should be six faces");
    std::cout << count[1] << " edges" << std::endl;
    test(count[1] == 0, "There should be zero edges");
    std::cout << count[0] << " vertex groups" << std::endl;
    test(count[0] == 0, "There should be zero vertex groups");

    bool debug = false;
    if (debug)
    {
      VisualizeModel(model->referenceAs<smtk::model::Model>());
    }

    return 0;
  }
}

int TestCreateUniformGridOp(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  return Test2DGrid() + Test3DGrid();
}
