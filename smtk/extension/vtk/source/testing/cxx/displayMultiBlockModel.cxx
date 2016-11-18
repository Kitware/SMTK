//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/ImportJSON.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include "smtk/model/Manager.h"

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

using smtk::shared_ptr;
using namespace smtk::common;
using namespace smtk::model;
using namespace smtk::io;

int main(int argc, char* argv[])
{
  int debug = argc > 2 ? (argv[2][0] == '-' ? 0 : 1) : 0;
  std::ifstream file;
  if (argc > 1)
    file.open(argv[1]);
  if (argc < 2 || !file.good())
    {
    cout
      << "Could not open file \"" << (argc > 1 ? argv[1] : "smtkModel.json") << "\".\n\n"
      << "Usage:\n  " << argv[0] << " [filename [debug|test_options]]\n"
      << "where\n"
      << "  filename is the path to a JSON model.\n"
      << "  debug    is any character other than '-'; its presence turns the test into an interactive demo.\n"
      << "and test_options is some combination of the following\n"
      << "  -T tmpdir is a temporary directory to store baselines from tests.\n"
      << "  -V image  is the path to a baseline image for filename.\n"
      ;
    return 1;
    }
  if (debug)
    {
    cout
      << "\n\n"
      << "Press 'q' to exit\n\n"
      ;
    }
  std::string data(
    (std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));

  ManagerPtr sm = Manager::create();

  int status = ! ImportJSON::intoModelManager(data.c_str(), sm);
  if (! status)
    {
    vtkNew<vtkActor> act;
    vtkNew<vtkModelMultiBlockSource> src;
    vtkNew<vtkCompositePolyDataMapper> map;
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> win;
    src->SetModelManager(sm);
    // TODO: Accept a model name or UUID and tell the src to use it:
    // src->SetModelEntityID(
    //   sm->entitiesMatchingFlagsAs<Models>(MODEL_ENTITY)[0].entity().toString().c_str());
    if (debug)
      win->SetMultiSamples(16);
    if (debug && argv[2][0] != '0')
      src->AllowNormalGenerationOn();
    map->SetInputConnection(src->GetOutputPort());
    act->SetMapper(map.GetPointer());
    act->GetProperty()->SetPointSize(5);
    act->GetProperty()->SetLineWidth(2);

    win->AddRenderer(ren.GetPointer());
    ren->AddActor(act.GetPointer());

    vtkRenderWindowInteractor* iac = win->MakeRenderWindowInteractor();
    vtkInteractorStyleSwitch::SafeDownCast(iac->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    win->SetInteractor(iac);

    if (debug && argc > 3)
      {
      vtkNew<vtkXMLMultiBlockDataWriter> wri;
      wri->SetInputConnection(src->GetOutputPort());
      wri->SetFileName(argv[3]);
      wri->Write();
      }

    win->Render();
    ren->ResetCamera();

    status = ! vtkRegressionTestImage(win.GetPointer());

    if (debug)
      {
      iac->Start();
      }
    }

  return status;
}
