//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBModelMapper - Mapper for a CMB model.
// .SECTION Description
// vtkCMBModelMapper iterates over the geometry entities in a model internally.

#ifndef __vtkCMBModelMapper_h
#define __vtkCMBModelMapper_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkCompositePolyDataMapper2.h"
#include "cmbSystemConfig.h"

class vtkProperty;
class vtkDiscreteModelWrapper;
class vtkWindow;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBModelMapper : public vtkCompositePolyDataMapper2
{
public:
  static vtkCMBModelMapper* New();
  vtkTypeMacro(vtkCMBModelMapper, vtkCompositePolyDataMapper2);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Implemented by sub classes. Actual rendering is done here.

  virtual void RenderPiece(vtkRenderer *ren, vtkActor *act);
  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // The cmb model that contains all the CMB model APIs.
  vtkGetObjectMacro(CMBModel, vtkDiscreteModelWrapper);
  void SetCMBModel(vtkDiscreteModelWrapper* model);

  // Description:
  // Flag to show the edge points of a 2D model. Default is false
  vtkBooleanMacro(ShowEdgePoints, bool);
  vtkSetMacro(ShowEdgePoints, bool);
  vtkGetMacro(ShowEdgePoints, bool);

protected:
  vtkCMBModelMapper();
  ~vtkCMBModelMapper();

  // Description:
  // Need to define the type of data handled by this mapper.
  virtual int FillInputPortInformation(int port, vtkInformation* info);
  // see algorithm for more info
  virtual int FillOutputPortInformation(int port, vtkInformation* info);

  // Description:
  // Calling rendering for the painter
  virtual void RenderInternal(vtkDataObject* inputObj,
    vtkRenderer* renderer, vtkActor* actor,
    unsigned long typeflags, bool forceCompileOnly);

  // Description:
  // Need to loop over all visible entities to compute bounds
  virtual void ComputeBounds();

  // Description:
  // Time stamp for computation of bounds.
  vtkTimeStamp BoundsMTime;

  vtkDiscreteModelWrapper* CMBModel;
  int ColorBlocks;
  bool ShowEdgePoints;
  int ModelDisplayListId;

private:
  vtkCMBModelMapper(const vtkCMBModelMapper&); // Not implemented.
  void operator=(const vtkCMBModelMapper&); // Not implemented.

};

#endif
