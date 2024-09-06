//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/source/vtkConeFrustum.h"
#include "smtk/extension/vtk/widgets/vtkConeRepresentation.h"
#include "smtk/extension/vtk/widgets/vtkConeWidget.h"

#include "smtk/model/Resource.h"
#include "smtk/model/json/jsonResource.h"

#include "vtkActor.h"
#include "vtkCommand.h"
#include "vtkCompositePolyDataMapper.h"
#include "vtkCubeSource.h"
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
#include "vtkXMLPolyDataWriter.h"

#include "vtkRegressionTestImage.h"

using namespace smtk::common;
using namespace smtk::model;
using namespace smtk::io;

int main(int argc, char* argv[])
{
  bool debug = argc > 1;
  vtkNew<vtkRenderer> ren;
  vtkNew<vtkRenderWindow> win;
  if (debug)
  {
    win->SetMultiSamples(16);
  }
  else
  {
    win->SetMultiSamples(0);
  }

  ren->SetBackground(0.9, 0.9, 0.9);
  win->AddRenderer(ren.GetPointer());
  win->SetSize(300, 300);

  vtkRenderWindowInteractor* iac = win->MakeRenderWindowInteractor();
  vtkInteractorStyleSwitch::SafeDownCast(iac->GetInteractorStyle())
    ->SetCurrentStyleToTrackballCamera();
  win->SetInteractor(iac);

  vtkNew<vtkCubeSource> cube;
  vtkNew<vtkActor> cubeAct;
  vtkNew<vtkPolyDataMapper> cubeMap;
  cube->SetCenter(1, 0, 0);
  cube->SetXLength(0.5);
  cube->SetYLength(0.5);
  cube->SetZLength(0.5);
  cubeAct->SetMapper(cubeMap);
  cubeMap->SetInputConnection(cube->GetOutputPort());
  ren->AddActor(cubeAct);

  vtkNew<vtkConeWidget> widget;
  widget->SetInteractor(iac);
  widget->SetEnabled(true);
  // To test cylinder editing, uncomment this:
  // reinterpret_cast<vtkConeRepresentation*>(widget->GetRepresentation())->CylindricalOn();

#if 0
  // Handy for debugging:
  if (debug && argc > 3)
  {
    vtkNew<vtkXMLPolyDataWriter> wri;
    wri->SetInputConnection(src->GetOutputPort());
    wri->SetFileName(argv[3]);
    wri->Write();
  }
#endif

  ren->ResetCamera();
  win->Render();

  int status = !vtkRegressionTestImage(win.GetPointer());

  if (debug)
  {
    iac->Start();
  }

  return status;
}
