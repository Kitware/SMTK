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
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/attribute/VoidItem.h"
#include "smtk/model/Edge.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/RegisterOperations.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/operators/AddAuxiliaryGeometry.h"

#include "smtk/operation/LoadResource.h"
#include "smtk/operation/RegisterOperations.h"
#include "smtk/operation/SaveResource.h"

#include "smtk/bridge/polygon/RegisterSession.h"
#include "smtk/bridge/polygon/Resource.h"
#include "smtk/bridge/polygon/operators/Delete.h"
#include "smtk/bridge/polygon/operators/Import.h"

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
    smtk::bridge::polygon::registerResources(resourceManager);
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  {
    smtk::operation::registerOperations(operationManager);
    smtk::model::registerOperations(operationManager);
    smtk::bridge::polygon::registerOperations(operationManager);
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  smtk::operation::LoadResource::Ptr loadOp =
    operationManager->create<smtk::operation::LoadResource>();

  loadOp->parameters()->findFile("filename")->setValue(std::string(argv[1]));

  std::cout << "Importing " << argv[1] << "\n";

  smtk::operation::NewOp::Result loadOpResult = loadOp->operate();
  test(loadOpResult->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED),
    "Load operator failed");

  smtk::bridge::polygon::Resource::Ptr manager =
    smtk::dynamic_pointer_cast<smtk::bridge::polygon::Resource>(
      loadOpResult->findResource("resource")->value(0));

  smtk::model::Models models =
    manager->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);

  if (models.size() < 1)
    return 1;

  smtk::model::Model simpleSMTK = models[0];

  if (!simpleSMTK.isValid())
  {
    std::cerr << "Reading model " << argv[1] << " file failed!\n";
    return 1;
  }

  // get edge info
  EntityRefs edges = manager->entitiesMatchingFlagsAs<EntityRefs>(smtk::model::EDGE);
  std::cout << "Edges inside model are:\n";
  for (auto edge : edges)
  {
    std::cout << " " << edge.name() << "\n";
  }

  // start delete operator
  std::cout << "Create the delete operator\n";
  smtk::bridge::polygon::Delete::Ptr deleteOp =
    operationManager->create<smtk::bridge::polygon::Delete>();
  if (!deleteOp)
  {
    std::cout << "No delete operator\n";
    return 1;
  }

  smtk::model::Edge edge0 = manager->findEntitiesByPropertyAs<Edges>("name", "edge 0")[0];
  test(edge0.isValid(), "edge 0 is not valid!\n");
  smtk::model::Edge edge1 = manager->findEntitiesByPropertyAs<Edges>("name", "edge 1")[0];
  test(edge1.isValid(), "edge 1 is not valid!\n");

  bool result(0);
  result = deleteOp->parameters()->associateEntity(edge0);
  test(result == 1);
  result = deleteOp->parameters()->associateEntity(edge1);
  test(result == 1);

  // it's designed to fail.
  smtk::bridge::polygon::Delete::Result deleteOpResult = deleteOp->operate();
  if (deleteOpResult->findInt("outcome")->value() ==
    static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED))
  {
    std::cerr << "Delete operator should not succeed!\n";
    return 1;
  }
  return 0;
}
