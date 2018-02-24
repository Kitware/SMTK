//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/mesh/Resource.h"
#include "smtk/bridge/mesh/operators/ImportOperation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/mesh/core/Manager.h"
#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"

#include "smtk/operation/Manager.h"
#include "smtk/resource/Manager.h"

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
  std::string dataRoot = SMTK_DATA_DIR;
  std::string writeRoot = SMTK_SCRATCH_DIR;

  int ImportModel(smtk::model::Entity::Ptr & model,
    smtk::operation::Manager::Ptr & operationManager, const std::string& filename,
    std::string label = std::string())
  {
    {
      smtk::bridge::mesh::ImportOperation::Ptr importOp =
        operationManager->create<smtk::bridge::mesh::ImportOperation>();
      if (!importOp)
      {
        std::cerr << "No import operator\n";
        return 1;
      }

      importOp->parameters()->findFile("filename")->setValue(filename);
      importOp->parameters()->findString("label")->setValue(label);
      importOp->parameters()->findVoid("construct hierarchy")->setIsEnabled(true);

      smtk::bridge::mesh::ImportOperation::Result importOpResult = importOp->operate();

      smtk::attribute::ComponentItemPtr componentItem =
        std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
          importOpResult->findComponent("model"));

      model = std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

      if (importOpResult->findInt("outcome")->value() !=
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
      {
        std::cerr << "Import operator failed\n";
        return 1;
      }
    }
    return 0;
  }

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
#else
    (void)model;
#endif
  }
}

int UnitTestTopology(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Register the mesh resource to the resource manager
  {
    resourceManager->registerResource<smtk::bridge::mesh::Resource>();
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register import operator to the operation manager
  {
    operationManager->registerOperation<smtk::bridge::mesh::ImportOperation>(
      "smtk::bridge::mesh::ImportOperation");
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  {
    smtk::model::Entity::Ptr model;

    std::string readFilePath(dataRoot);
    readFilePath += "/model/3d/exodus/SimpleReactorCore/SimpleReactorCore.exo";

    test(ImportModel(model, operationManager, readFilePath) == 0,
      "Could not import model " + readFilePath);

    std::size_t count[4] = { 0, 0, 0, 0 };
    ParseModelTopology(model->referenceAs<smtk::model::Model>(), count);

    std::cout << count[3] << " volumes" << std::endl;
    test(count[3] == 3, "There should be three volumes");
    std::cout << count[2] << " faces" << std::endl;
    test(count[2] == 9, "There should be nine faces");
    std::cout << count[1] << " edges" << std::endl;
    test(count[1] == 7, "There should be seven lines");
    std::cout << count[0] << " vertex groups" << std::endl;
    test(count[0] == 1, "There should be one vertex group");

    bool debug = false;
    if (debug)
    {
      VisualizeModel(model->referenceAs<smtk::model::Model>());
    }
  }

  {
    smtk::model::Entity::Ptr model;

    std::string readFilePath(dataRoot);
    readFilePath += "/model/3d/genesis/gun-1fourth.gen";

    test(ImportModel(model, operationManager, readFilePath) == 0,
      "Could not import model " + readFilePath);

    std::size_t count[4] = { 0, 0, 0, 0 };
    ParseModelTopology(model->referenceAs<smtk::model::Model>(), count);

    std::cout << count[3] << " volumes" << std::endl;
    test(count[3] == 1, "There should be one volume");
    std::cout << count[2] << " faces" << std::endl;
    test(count[2] == 5, "There should be five faces");
    std::cout << count[1] << " edges" << std::endl;
    test(count[1] == 9, "There should be nine lines");
    std::cout << count[0] << " vertex groups" << std::endl;
    test(count[0] == 6, "There should be six vertex groups");

    bool debug = false;
    if (debug)
    {
      VisualizeModel(model->referenceAs<smtk::model::Model>());
    }
  }

#ifdef SMTK_ENABLE_VTK_SUPPORT
  {
    smtk::model::Entity::Ptr model;

    std::string readFilePath(dataRoot);
    readFilePath += "/mesh/3d/nickel_superalloy.vtu";

    test(ImportModel(model, operationManager, readFilePath, "ZoneIds") == 0,
      "Could not import model " + readFilePath);

    std::size_t count[4] = { 0, 0, 0, 0 };
    ParseModelTopology(model->referenceAs<smtk::model::Model>(), count);

    std::cout << count[3] << " volumes" << std::endl;
    test(count[3] == 5, "There should be five volumes");
    std::cout << count[2] << " faces" << std::endl;
    test(count[2] == 12, "There should be twelve faces");
    std::cout << count[1] << " edges" << std::endl;
    test(count[1] == 10, "There should be ten lines");
    std::cout << count[0] << " vertex groups" << std::endl;
    test(count[0] == 3, "There should be three vertex groups");

    bool debug = false;
    if (debug)
    {
      VisualizeModel(model->referenceAs<smtk::model::Model>());
    }
  }
#endif

  return 0;
}
