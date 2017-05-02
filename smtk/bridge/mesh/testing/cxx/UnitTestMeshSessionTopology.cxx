//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/mesh/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/mesh/Manager.h"
#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"

#include <chrono>

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
smtkComponentInitMacro(smtk_extension_vtk_io_MeshIOVTK)
#endif

  namespace
{
  std::string dataRoot = SMTK_DATA_DIR;
  std::string writeRoot = SMTK_SCRATCH_DIR;

  int ImportModel(smtk::model::Model & model, smtk::model::SessionRef & session,
    const std::string& filename, std::string label = std::string())
  {
    {
      smtk::model::OperatorPtr importOp = session.op("import");
      if (!importOp)
      {
        std::cerr << "No import operator\n";
        return 1;
      }

      importOp->specification()->findFile("filename")->setValue(filename);
      importOp->specification()->findString("label")->setValue(label);

      std::chrono::time_point<std::chrono::system_clock> start, end;
      start = std::chrono::system_clock::now();

      smtk::model::OperatorResult importOpResult = importOp->operate();

      end = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_seconds = end - start;
      std::cout << "elapsed time: " << elapsed_seconds.count() << "s" << std::endl;

      model = importOpResult->findModelEntity("model")->value();

      if (importOpResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
      {
        std::cerr << "Read operator failed\n";
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

  void ParseModelTopology(smtk::model::Model & model, std::size_t * count)
  {
    smtk::mesh::CollectionPtr collection = model.manager()->meshes()->collection(model.entity());

    std::set<smtk::model::EntityRef> unique;
    UniqueEntities(model, unique);

    for (auto&& entity : unique)
    {
      if (entity.dimension() >= 0 && entity.dimension() <= 3)
      {
        count[entity.dimension()]++;
        float r = static_cast<float>(entity.dimension()) / 3;
        float b = 1. - r;
        const_cast<smtk::model::EntityRef&>(entity).setColor(
          (r < 1. ? r : 1.), 0., (b < 1. ? b : 1.), 1.);
      }
    }
  }

  void VisualizeModel(smtk::model::Model & model)
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

int UnitTestMeshSessionTopology(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  smtk::model::ManagerPtr manager = smtk::model::Manager::create();

  smtk::model::SessionRef session = manager->createSession("mesh");

  {
    smtk::model::Model model;

    std::string readFilePath(dataRoot);
    readFilePath += "/model/3d/exodus/SimpleReactorCore/SimpleReactorCore.exo";

    test(ImportModel(model, session, readFilePath) == 0, "Could not import model " + readFilePath);

    std::size_t count[4] = { 0, 0, 0, 0 };
    ParseModelTopology(model, count);

    std::cout << count[3] << " volumes" << std::endl;
    test(count[3] == 3, "There should be three volumes");
    std::cout << count[2] << " faces" << std::endl;
    test(count[2] == 5, "There should be five faces");
    std::cout << count[1] << " edges" << std::endl;
    test(count[1] == 2, "There should be two lines");
    std::cout << count[0] << " vertex groups" << std::endl;
    test(count[0] == 0, "There should be no vertex groups");

    bool debug = false;
    if (debug)
    {
      VisualizeModel(model);
    }
  }

#ifdef SMTK_ENABLE_VTK_SUPPORT
  {
    smtk::model::Model model;

    std::string readFilePath(dataRoot);
    readFilePath += "/mesh/3d/nickel_superalloy.vtu";

    test(ImportModel(model, session, readFilePath, "ZoneIds") == 0,
      "Could not import model " + readFilePath);

    std::size_t count[4] = { 0, 0, 0, 0 };
    ParseModelTopology(model, count);

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
      VisualizeModel(model);
    }
  }
#endif

  return 0;
}
