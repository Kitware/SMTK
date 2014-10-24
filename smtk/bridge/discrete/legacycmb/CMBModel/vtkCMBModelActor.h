//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelActor - an actor that supports cmb model
// .SECTION Description
// vtkCMBModelActor  is a very simple version of vtkPVLODActor.
// It overwrite the RenderOpaqueGeometry, RenderTranslucentPolygonalGeometry
// .SECTION see also
// vtkActor vtkPVLODActor vtkRenderer vtkLODProp3D vtkLODActor

#ifndef __vtkCMBModelActor_h
#define __vtkCMBModelActor_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkPVLODActor.h"
#include "cmbSystemConfig.h"

class vtkViewport;
class vtkWindow;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBModelActor : public vtkPVLODActor
{
public:
  vtkTypeMacro(vtkCMBModelActor,vtkPVLODActor);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkCMBModelActor *New();

  // Description:
  // This causes the actor to be rendered. It, in turn, will render the actor's
  // property and then mapper.
  virtual void Render(vtkRenderer *, vtkMapper *);

  // Description:
  // Support the standard render methods.
  virtual int RenderOpaqueGeometry(vtkViewport *viewport);

protected:
  vtkCMBModelActor();
  ~vtkCMBModelActor();

  // Description:
  // Pre/Post process for model rendering
  virtual void PreModelRender(vtkRenderer *ren);
  virtual void PostModelRender(vtkRenderer *ren);

private:
  vtkCMBModelActor(const vtkCMBModelActor&); // Not implemented.
  void operator=(const vtkCMBModelActor&); // Not implemented.
};

#endif