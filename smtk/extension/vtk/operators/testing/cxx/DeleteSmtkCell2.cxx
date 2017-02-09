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

  // get edge info
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

  smtk::model::Edge edge0 = manager->findEntitiesByPropertyAs<Edges>("name",
             "model 0, edge 0")[0];
  test(edge0.isValid(), "edge 0 is not valid!\n");
  smtk::model::Edge edge1 = manager->findEntitiesByPropertyAs<Edges>("name",
             "model 0, edge 1")[0];
  test(edge1.isValid(), "edge 1 is not valid!\n");

  bool result(0);
  result = deleteOp->specification()->associateEntity(edge0);
  test(result == 1);
  result = deleteOp->specification()->associateEntity(edge1);
  test(result == 1);

  // it's designed to fail.
  smtk::model::OperatorResult deleteOpResult = deleteOp->operate();
  if (deleteOpResult->findInt("outcome")->value() ==
            smtk::model::OPERATION_SUCCEEDED)
  {
    std::cerr << "Delete operator should not success!\n";
    return 1;
  }
  return 0;
}

// This macro ensures the vtk io library is loaded into the executable
smtkComponentInitMacro(smtk_apply_bathymetry_operator)
