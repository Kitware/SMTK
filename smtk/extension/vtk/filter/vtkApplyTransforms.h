//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_vtk_ApplyTransforms_h
#define smtk_vtk_ApplyTransforms_h

#include "smtk/extension/vtk/filter/vtkSMTKFilterExtModule.h" // For export macro
#include "vtkCompositeDataSetAlgorithm.h"
#include "vtkNew.h"

class vtkDataSetSurfaceFilter;
class vtkTransform;
class vtkTransformFilter;

/**\brief Apply per-dataset transforms held in field data to the input collection as well
 * as extract the boundary of volumetric datasets.
  *
  */
class VTKSMTKFILTEREXT_EXPORT vtkApplyTransforms : public vtkCompositeDataSetAlgorithm
{
public:
  static vtkApplyTransforms* New();
  vtkTypeMacro(vtkApplyTransforms, vtkCompositeDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkApplyTransforms(const vtkApplyTransforms&) = delete;
  vtkApplyTransforms& operator=(const vtkApplyTransforms&) = delete;

protected:
  vtkApplyTransforms();
  ~vtkApplyTransforms() override = default;

  int FillOutputPortInformation(int port, vtkInformation* info) override;
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  vtkNew<vtkDataSetSurfaceFilter> SurfaceFilter;
  vtkNew<vtkTransformFilter> TransformFilter;
  vtkNew<vtkTransform> Transform;
};

#endif // smtk_vtk_ApplyTransforms_h
