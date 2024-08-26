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

#include "smtk/extension/vtk/source/vtkResourceMultiBlockSource.h"

#include "smtk/geometry/Manager.h"

#include "smtk/session/mesh/Registrar.h"
#include "smtk/session/mesh/Session.h"
#include "smtk/session/mesh/operators/CreateUniformGrid.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Resource.h"

#include "smtk/resource/Manager.h"

#include "smtk/operation/Manager.h"

#include "smtk/plugin/Registry.h"

#include <QApplication>
#include <QHeaderView>
#include <QLayout>
#include <QTimer>
#include <QTreeView>

#include <vtkActor.h>
#include <vtkCompositePolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>

#include <QVTKOpenGLNativeWidget.h>

#include "ModelViewer.h"

#include <fstream>
#include <iomanip>
#include <iostream>

#include <stdlib.h>

// This macro ensures the vtk io library is loaded into the executable
smtkComponentInitMacro(smtk_extension_vtk_io_mesh_MeshIOVTK);

int main(int argc, char* argv[])
{
  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Create a geometry manager
  smtk::geometry::Manager::Ptr geometryManager = smtk::geometry::Manager::create();

  // Register mesh resources to the resource, operation, and geomety managers.
  auto registry = smtk::plugin::addToManagers<smtk::session::mesh::Registrar>(
    resourceManager, operationManager, geometryManager);

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

  createBackgroundDomainOp->parameters()->findString("dimension")->setValue("3");
  createBackgroundDomainOp->parameters()->findDouble("size3d")->setValue(0, 2);

  createBackgroundDomainOp->parameters()->findInt("discretization3d")->setValue(0, 5);
  createBackgroundDomainOp->parameters()->findInt("discretization3d")->setValue(1, 5);
  createBackgroundDomainOp->parameters()->findInt("discretization3d")->setValue(2, 5);

  smtk::operation::Operation::Result createBackgroundDomainOpResult =
    createBackgroundDomainOp->operate();

  if (
    createBackgroundDomainOpResult->findInt("outcome")->value() !=
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

  smtk::resource::Resource::Ptr resource = model->resource();

  QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
  QApplication app(argc, argv);

  vtkSmartPointer<vtkResourceMultiBlockSource> generator =
    vtkSmartPointer<vtkResourceMultiBlockSource>::New();
  generator->SetResource(resource);
  generator->Update();

  ModelViewer modelViewer;

  QVTKOpenGLNativeWidget widget;
  widget.resize(256, 256);

  vtkSmartPointer<vtkCompositePolyDataMapper> sphereMapper =
    vtkSmartPointer<vtkCompositePolyDataMapper>::New();
  sphereMapper->SetInputConnection(generator->GetOutputPort());

  vtkSmartPointer<vtkActor> sphereActor = vtkSmartPointer<vtkActor>::New();
  sphereActor->SetMapper(sphereMapper);

  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  renderer->AddActor(sphereActor);

  widget.renderWindow()->AddRenderer(renderer);

  modelViewer.layout()->addWidget(&widget);

  modelViewer.show();

  int status = app.exec();

  return status;
}
