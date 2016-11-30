//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/Face.h"
#include "smtk/model/Model.h"
#include "smtk/model/Group.h"
#include "smtk/model/SimpleModelSubphrases.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Tessellation.h"

#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"
#include "vtkActor.h"
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

  smtk::bridge::discrete::Session::Ptr session = smtk::bridge::discrete::Session::create();
  manager->registerSession(session);

  std::cout << "Available cmb operators in discrete session\n";
  StringList opnames = session->operatorNames();
  for (StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  // read the data
  smtk::model::OperatorPtr readOp = session->op("read");
  if (!readOp)
    {
    std::cerr << "No read operator\n";
    return 1;
    }

  readOp->specification()->findFile("filename")->setValue(std::string(argv[1]));
  std::cout << "Importing " << argv[1] << "\n";
  smtk::model::OperatorResult opresult = readOp->operate();
  if (
    opresult->findInt("outcome")->value() !=
    smtk::model::OPERATION_SUCCEEDED)
    {
    std::cerr << "Read operator failed\n";
    return 1;
    }

  // create the bathymetry operator
  std::cout <<  "Creating apply bathymetry operator\n";
  smtk::model::OperatorPtr bathyOperator =  session->op("apply bathymetry");
  if (!bathyOperator)
  {
    std::cerr << "No apply bathymetry!\n";
    return 1;
  }
  // Check the optypeItem value

  std::cout<<"optypeItem initial value is: "<< bathyOperator->specification()->findString("operation")->value() << std::endl;

  // assign model value
  smtk::model::Model modelChesaBay = opresult->findModelEntity("created")->value();
  manager->assignDefaultNames(); // should force transcription of every entity, but doesn't yet.

  if (!modelChesaBay.isValid())
  {
    std::cerr << "Reading bay contour file failed!\n";
    return 1;
  }

  // set input values for bathymetry filter
  bathyOperator->specification()->findModelEntity("model")->setValue(modelChesaBay);
  bathyOperator->specification()->findFile("bathymetryfile")->setValue(std::string(argv[2]));
  bathyOperator->specification()->findDouble("averaging elevation radius")->setValue(0.05);

  smtk::model::OperatorResult bathyResult = bathyOperator->operate();
  if (bathyResult->findInt("outcome")->value() !=
    smtk::model::OPERATION_SUCCEEDED)
  {
    std::cerr <<"Apply bathymetry operator failed\n";
    return 1;
  }

  // Remove the bathymetry operator test
  //bathyOperator->specification()->findString("operation")->setValue("Remove Bathymetry");
  //std::cout<<"optypeItem value in Bathymetry Operator is: "<< bathyOperator->specification()->findString("operation")->value() << std::endl;
  //smtk::model::OperatorResult RmBathyResult = bathyOperator->operate();

  //if (RmBathyResult->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
  //{
  //  std::cerr<< "Remove Bathymetry operator failed\n";
  //  return 1;
  //}

  vtkNew<vtkActor> act;
  vtkNew<vtkModelMultiBlockSource> src;
  vtkNew<vtkCompositePolyDataMapper> map;
  vtkNew<vtkRenderer> ren;
  vtkNew<vtkRenderWindow> win;
  src->SetModelManager(manager);
  win->SetMultiSamples(16);
  src->AllowNormalGenerationOn();
  map->SetInputConnection(src->GetOutputPort());
  act->SetMapper(map.GetPointer());
  act->SetScale(100,100,1);

  win->AddRenderer(ren.GetPointer());
  ren->AddActor(act.GetPointer());
  ren->SetBackground(0.5, 0.5, 1);
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
