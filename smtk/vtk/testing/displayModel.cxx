#include "smtk/model/ImportJSON.h"
#include "smtk/model/ModelBody.h"
#include "smtk/vtk/vtkSMTKModelRepresentation.h"
#include "smtk/vtk/vtkSMTKModelSource.h"
#include "smtk/vtk/vtkSMTKModelView.h"

#include "vtkInteractorStyleSwitch.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderedSurfaceRepresentation.h"
#include "vtkPolyDataWriter.h"

using namespace smtk::model;
using namespace smtk::util;

int main(int argc, char* argv[])
{
  int debug = argc > 2 ? 1 : 0;
  std::ifstream file(argc > 1 ? argv[1] : "smtkModel.json");
  std::string data(
    (std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));

  UUIDsToLinks smTopology;
  UUIDsToArrangements smArrangements;
  UUIDsToTessellations smTessellation;
  ModelBodyPtr sm = ModelBodyPtr(new ModelBody(&smTopology, &smArrangements, &smTessellation));

  int status = 0;
  status |= ImportJSON::intoModel(data.c_str(), sm.get());
  vtkNew<vtkSMTKModelView> view;
  vtkNew<vtkSMTKModelSource> src;
  vtkNew<vtkSMTKModelRepresentation> rep;
  src->SetModel(sm);
  rep->SetInputConnection(src->GetOutputPort());

  view->AddRepresentation(rep.GetPointer());
  view->ResetCamera();
  view->ResetCameraClippingRange();
  //view->SetInteractionMode(vtkRenderView::INTERACTION_MODE_3D);
  //vtkNew<vtkInteractorStyleSwitch> istyle;
  //view->SetInteractorStyle(istyle.GetPointer());

  //vtkRenderWindowInteractor* iac = view->GetRenderWindow()->MakeRenderWindowInteractor();
  //view->GetRenderWindow()->SetInteractor(iac);
  view->Render();
  view->ResetCamera();
  view->ResetCameraClippingRange();

#if 0
  // Using legacy writer... XML format doesn't deal well with string arrays (UUIDs).
  vtkNew<vtkPolyDataWriter> wri;
  wri->SetFileName("/tmp/smtkModel.vtk");
  wri->SetInputDataObject(src->GetOutput());
  //wri->SetDataModeToAscii();
  wri->Write();
#endif // 0

  if (debug)
    {
    view->GetInteractor()->Start();
    }

  return status;
}
