#ifndef __smtk_vtk_ModelView_h
#define __smtk_vtk_ModelView_h

#include "smtk/vtkSMTKExports.h"
#include "vtkRenderView.h"

class VTKSMTK_EXPORT vtkSMTKModelView : public vtkRenderView
{
public:
  static vtkSMTKModelView* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkSMTKModelView,vtkRenderView);

protected:
  vtkSMTKModelView();
  virtual ~vtkSMTKModelView();

private:
  vtkSMTKModelView(const vtkSMTKModelView&); // Not implemented.
  void operator = (const vtkSMTKModelView&); // Not implemented.
};

#endif // __smtk_vtk_ModelView_h
