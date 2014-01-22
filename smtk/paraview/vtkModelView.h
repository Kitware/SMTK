#ifndef __smtk_paraview_ModelView_h
#define __smtk_paraview_ModelView_h

#include "smtk/pvSMTKExports.h"
#include "vtkRenderView.h"

namespace smtk {
  namespace model {

/**\brief A vtkView subclass for displaying SMTK model geometry.
  *
  * This currently only works with the vtkPolyData-style vtkModelSource class.
  */
class PVSMTK_EXPORT vtkModelView : public vtkRenderView
{
public:
  static vtkModelView* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkModelView,vtkRenderView);

protected:
  vtkModelView();
  virtual ~vtkModelView();

private:
  vtkModelView(const vtkModelView&); // Not implemented.
  void operator = (const vtkModelView&); // Not implemented.
};

  } // namespace model
} // namespace smtk

#endif // __smtk_paraview_ModelView_h
