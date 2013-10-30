#include "smtk/model/ImportJSON.h"
#include "smtk/model/ModelBody.h"
#include "smtk/vtk/vtkSMTKModelRepresentation.h"
#include "smtk/vtk/vtkSMTKModelView.h"

#include "vtkInteractorStyleSwitch.h"
#include "vtkNew.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkXMLPolyDataWriter.h"

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
  vtkNew<vtkSMTKModelRepresentation> rep;
  rep->SetModel(sm);

#if 0
  vtkNew<vtkXMLPolyDataWriter> wri;
  wri->SetFileName("/tmp/smtkModel.vtp");
  wri->SetInputConnection(rep->GetOutputPort());
  wri->SetDataModeToAscii();
  wri->Write();
#endif // 0

  view->AddRepresentation(rep.GetPointer());
  view->ResetCamera();
  view->ResetCameraClippingRange();
  //view->SetInteractionMode(vtkRenderView::INTERACTION_MODE_3D);
  //vtkNew<vtkInteractorStyleSwitch> istyle;
  //view->SetInteractorStyle(istyle.GetPointer());
  vtkRenderWindowInteractor* iac = view->GetRenderWindow()->MakeRenderWindowInteractor();
  view->GetRenderWindow()->SetInteractor(iac);
  view->Render();
  view->ResetCamera();
  view->ResetCameraClippingRange();

  if (debug)
    {
    view->GetInteractor()->Start();
    }

  return status;
}
