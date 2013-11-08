#include "smtk/vtk/vtkSMTKModelView.h"

#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkSMTKModelView);

vtkSMTKModelView::vtkSMTKModelView()
{
}

vtkSMTKModelView::~vtkSMTKModelView()
{
  // Even though the base class will call this again, we must
  // call RemoveAllRepresentations() now.
  // Otherwise, representations which downcast the view pointer
  // to a subclass of vtkView (in order to call subclass methods)
  // will fail.
  this->RemoveAllRepresentations();
}

void vtkSMTKModelView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
