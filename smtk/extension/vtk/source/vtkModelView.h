//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_vtk_ModelView_h
#define __smtk_vtk_ModelView_h

#include "smtk/extension/vtk/source/Exports.h"
#include "vtkRenderView.h"

/**\brief A vtkView subclass for displaying SMTK model geometry.
  *
  * This currently only works with the vtkPolyData-style vtkModelSource class.
  */
class VTKSMTKSOURCEEXT_EXPORT vtkModelView : public vtkRenderView
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

#endif // __smtk_vtk_ModelView_h
