//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelSelectionPainter - painter that can be inserted before any
// vtkDataSet painting chain to handle cmb model.
// .SECTION Description
// vtkCMBModelSelectionPainter iterates over the leaves in a cmb model.

#ifndef __vtkCMBModelSelectionPainter_h
#define __vtkCMBModelSelectionPainter_h

#include "cmbSystemConfig.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkPainter.h"

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBModelSelectionPainter : public vtkPainter
{
public:
  static vtkCMBModelSelectionPainter* New();
  vtkTypeMacro(vtkCMBModelSelectionPainter, vtkPainter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the output data object from this painter. The default implementation
  // simply forwards the input data object as the output.
  virtual vtkDataObject* GetOutput();

protected:
  vtkCMBModelSelectionPainter();
  ~vtkCMBModelSelectionPainter();

  // Description:
  // Take part in garbage collection.
  virtual void ReportReferences(vtkGarbageCollector* collector);

  // Description:
  // Performs the actual rendering. Subclasses may override this method.
  // default implementation merely call a Render on the DelegatePainter,
  // if any. When RenderInternal() is called, it is assured that the
  // DelegatePainter is in sync with this painter i.e. UpdateDelegatePainter()
  // has been called.
  virtual void RenderInternal(
    vtkRenderer* renderer, vtkActor* actor, unsigned long typeflags, bool forceCompileOnly);

  vtkDataObject* OutputData;

private:
  vtkCMBModelSelectionPainter(const vtkCMBModelSelectionPainter&); // Not implemented.
  void operator=(const vtkCMBModelSelectionPainter&);              // Not implemented.
};

#endif
