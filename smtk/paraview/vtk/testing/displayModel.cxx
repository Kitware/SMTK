#include "smtk/model/ImportJSON.h"
#include "smtk/model/Storage.h"
#include "smtk/paraview/vtk/vtkModelRepresentation.h"
#include "smtk/paraview/vtk/vtkModelSource.h"
#include "smtk/paraview/vtk/vtkModelView.h"

#include "vtkAnnotationLink.h"
#include "vtkCommand.h"
#include "vtkDataSetAttributes.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataWriter.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderedSurfaceRepresentation.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkTextProperty.h"
#include "vtkViewTheme.h"

using smtk::shared_ptr;
using namespace smtk::model;
using namespace smtk::util;

void applyPublicationTheme(vtkModelView* view)
{
  vtkNew<vtkViewTheme> theme;
  theme->SetPointSize(7);
  theme->SetLineWidth(2);
  theme->SetBackgroundColor(1., 1., 1.);
  theme->SetBackgroundColor2(1., 1., 1.);
  theme->GetPointTextProperty()->SetColor(0, 0, 0);
  theme->GetCellTextProperty()->SetColor(.7, .7, 1);
  theme->SetPointColor(0.5, 0.5, 0.5);
  theme->SetPointHueRange(0.667, 0);
  theme->SetCellColor(0.85, 0.85, 0.85);
  theme->SetCellOpacity(1.);
  theme->SetCellHueRange(0.667, 0);
  theme->SetCellAlphaRange(1, 1);
  theme->SetCellValueRange(0.5, 1);
  theme->SetCellSaturationRange(0.5, 1);
  theme->SetOutlineColor(0.8, 0.4, 0.4);
  theme->SetSelectedPointColor(0.8, 0.6, 0.4);
  theme->SetSelectedCellColor(0.8, 0.6, 0.6);
  view->ApplyViewTheme(theme.GetPointer());
}

// An observer that prints out selected model faces and
// switches interaction modes when the "m" key is pressed.
// (By default, mouse motion moves the camera but this
//  is toggled to selecting model faces with the "m" key.)
class vtkModelSelectionHelper : public vtkCommand
{
public:
  static vtkModelSelectionHelper* New() { return new vtkModelSelectionHelper; }
  vtkModelSelectionHelper()
    {
    }
  ~vtkModelSelectionHelper()
    {
    }
  void PrintSelectionMask(int mask)
    {
    cout << "Selecting:"
      << (mask & smtk::model::DIMENSION_0 ? " Vertices" : "")
      << (mask & smtk::model::DIMENSION_1 ? " Edges" : "")
      << (mask & smtk::model::DIMENSION_2 ? " Faces" : "")
      << (mask & smtk::model::DIMENSION_3 ? " Volumes" : "")
      << "\n";
    }
  virtual void Execute(vtkObject* caller, unsigned long eventId, void* vtkNotUsed(callData))
    {
    /*
    cout
      << "Event " << eventId << " caller " << caller
      << " (" << caller->GetClassName() << ")"
      << " data " << callData << "\n";
     */
    if (eventId == vtkCommand::KeyPressEvent)
      {
      vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::SafeDownCast(caller);
      if (iren)
        {
        char key = iren->GetKeySym()[0];
        if (key == 'm')
          {
          this->SwitchInteractors();
          this->PrintSelectionMask(this->Representation->GetSelectionMask());
          }
        else if (key == 'd' || key == 'D')
          {
          int mask = this->Representation->GetSelectionMask();
          mask = (mask + 1) % (
            smtk::model::DIMENSION_0 +
            smtk::model::DIMENSION_1 +
            smtk::model::DIMENSION_2 +
            smtk::model::DIMENSION_3 +
            1);
          if (mask == 0) ++mask; // Don't allow "select nothing"
          this->Representation->SetSelectionMask(mask);
          this->PrintSelectionMask(mask);
          }
        else if (key == 'q' || key == 'e')
          {
          // A quirk of keeping 2 interactors around is that only one's
          // TerminateApp() method actually stops processing the event
          // loop (the one on which Start() was called).
          this->CameraInteractor->TerminateApp();
          }
        }
      }
    else if (eventId == vtkCommand::SelectionChangedEvent)
      {
      vtkSelection* selection = this->Representation->GetAnnotationLink()->GetCurrentSelection();
      vtkSelectionNode* node = selection->GetNode(0);
      if (
        selection && node &&
        selection->GetNumberOfNodes() == 1 &&
        node->GetContentType() == vtkSelectionNode::PEDIGREEIDS)
        {
        cout << "Selection changed to [\n";
        vtkIndent indent;
        vtkStringArray* uuids = vtkStringArray::SafeDownCast(
          node->GetSelectionData()->GetAbstractArray(0));
        if (uuids)
          {
          vtkIdType nids = uuids->GetNumberOfTuples();
          std::set<std::string> uniques;
          for (vtkIdType i = 0; i < nids; ++i)
            {
            uniques.insert(uuids->GetValue(i));
            }
          std::set<std::string>::iterator it;
          for (it = uniques.begin(); it != uniques.end(); ++it)
            {
            cout
              << indent << *it << "  "
              << (this->Storage ? this->Storage->name(smtk::util::UUID(*it)) : "--")
              << "\n";
            }
          }
        cout << "]\n";
        }
      else if (selection && ! node)
        {
        cout << "Selection cleared\n";
        }
      }
    }

  void SetSelectionInteractor(vtkRenderWindowInteractor* si)
    {
    if (this->SelectionInteractor == si)
      {
      return;
      }
    if (this->SelectionInteractor)
      {
      this->SelectionInteractor->RemoveObserver(this);
      }
    this->SelectionInteractor = si;
    if (this->SelectionInteractor)
      {
      this->SelectionInteractor->AddObserver(vtkCommand::KeyPressEvent, this);
      }
    }

  void SetCameraInteractor(vtkRenderWindowInteractor* si)
    {
    if (this->CameraInteractor == si)
      {
      return;
      }
    if (this->CameraInteractor)
      {
      this->CameraInteractor->RemoveObserver(this);
      }
    this->CameraInteractor = si;
    if (this->CameraInteractor)
      {
      this->CameraInteractor->AddObserver(vtkCommand::KeyPressEvent, this);
      }
    }

  void SetRenderWindow(vtkRenderWindow* rw)
    {
    if (this->RenderWindow == rw)
      {
      return;
      }
    this->RenderWindow = rw;
    }

  void SetRepresentation(vtkModelRepresentation* rep)
    {
    if (this->Representation == rep)
      {
      return;
      }
    this->Representation = rep;
    }

  void SetStorage(smtk::model::StoragePtr sm)
    {
    this->Storage = sm;
    }

  void SwitchInteractors()
    {
    if (this->RenderWindow->GetInteractor() == this->CameraInteractor)
      {
      this->RenderWindow->SetInteractor(this->SelectionInteractor);
      }
    else
      {
      this->RenderWindow->SetInteractor(this->CameraInteractor);
      }
    }

  vtkSmartPointer<vtkRenderWindowInteractor> GetSelectionInteractor()
    {
	    return this->SelectionInteractor;
    }

  vtkSmartPointer<vtkRenderWindowInteractor> GetCameraInteractor()
    {
	    return this->CameraInteractor;
    }

protected:
  vtkSmartPointer<vtkRenderWindowInteractor> CameraInteractor;
  vtkSmartPointer<vtkRenderWindowInteractor> SelectionInteractor;
  vtkSmartPointer<vtkRenderWindow> RenderWindow;
  vtkSmartPointer<vtkModelRepresentation> Representation;
  smtk::model::StoragePtr Storage;
};

int main(int argc, char* argv[])
{
  int debug = argc > 2 ? 1 : 0;
  std::ifstream file(argc > 1 ? argv[1] : "smtkModel.json");
  if (!file.good())
    {
    cout
      << "Could not open file \"" << (argc > 1 ? argv[1] : "smtkModel.json") << "\".\n\n"
      << "Usage:\n  " << argv[0] << " [[filename] debug]\n"
      << "where\n"
      << "  filename is the path to a JSON model.\n"
      << "  debug    is any character; its presence turns the test into an interactive demo.\n\n"
      ;
    return 1;
    }
  if (debug)
    {
    cout
      << "\n\n"
      << "Press 'm' to switch interaction modes (camera motion vs selection)\n"
      << "      'q' to exit\n\n"
      ;
    }
  std::string data(
    (std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));

  StoragePtr sm = smtk::model::Storage::create();

  int status = ! ImportJSON::intoModel(data.c_str(), sm);
  if (! status)
    {
    vtkNew<vtkModelView> view;
    vtkNew<vtkModelSource> src;
    vtkNew<vtkModelRepresentation> rep;
    vtkModelSelectionHelper* hlp = NULL;
    if (debug)
      {
      hlp = vtkModelSelectionHelper::New();
      }
    src->SetModel(sm);
    rep->SetModel(sm);
    rep->SetSelectionMask(smtk::model::DIMENSION_1);
    rep->SetInputConnection(src->GetOutputPort());
    rep->SetSelectionType(vtkSelectionNode::PEDIGREEIDS);

    view->AddRepresentation(rep.GetPointer());
    view->ResetCamera();
    view->ResetCameraClippingRange();

    if (hlp)
      {
      rep->AddObserver(vtkCommand::SelectionChangedEvent, hlp);
      hlp->SetSelectionInteractor(view->GetRenderWindow()->GetInteractor());
      vtkRenderWindowInteractor* iac = view->GetRenderWindow()->MakeRenderWindowInteractor();
      vtkInteractorStyleSwitch::SafeDownCast(iac->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
      view->GetRenderWindow()->SetInteractor(iac);
      hlp->SetCameraInteractor(iac);
      hlp->SetRenderWindow(view->GetRenderWindow());
      hlp->SetRepresentation(rep.GetPointer());
      hlp->SetStorage(sm);
      }

    view->Render();
    view->ResetCamera();
    view->ResetCameraClippingRange();

#if 0
    // Using legacy writer... XML format doesn't deal well with string arrays (UUIDs).
    vtkNew<vtkPolyDataWriter> wri;
    wri->SetFileName("/tmp/smtkModel.vtk");
    wri->SetInputDataObject(src->GetOutput());
    wri->Write();
#endif // 0

    if (debug)
      {
      sm->assignDefaultNames();
      applyPublicationTheme(view.GetPointer());
      view->GetInteractor()->Start();
      hlp->GetSelectionInteractor()->RemoveAllObservers();
      hlp->GetCameraInteractor()->RemoveAllObservers();
      hlp->SetCameraInteractor(NULL);
      hlp->SetRenderWindow(NULL);
      hlp->SetRepresentation(NULL);
      hlp->SetSelectionInteractor(NULL);
      hlp->Delete();
      }
    view->RemoveAllRepresentations();
    }

  return status;
}
