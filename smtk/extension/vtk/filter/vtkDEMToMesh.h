//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_vtk_DEMToMesh_h
#define smtk_vtk_DEMToMesh_h

#include "smtk/extension/vtk/filter/vtkSMTKFilterExtModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class VTKSMTKFILTEREXT_EXPORT vtkDEMToMesh : public vtkPolyDataAlgorithm
{
public:
  static vtkDEMToMesh* New();
  vtkTypeMacro(vtkDEMToMesh, vtkPolyDataAlgorithm);

  vtkDEMToMesh(const vtkDEMToMesh&) = delete;
  vtkDEMToMesh& operator=(const vtkDEMToMesh&) = delete;

  void SetUseScalerForZ(int v);

protected:
  vtkDEMToMesh();
  ~vtkDEMToMesh() override;

  int FillInputPortInformation(int port, vtkInformation* info) override;

  int RequestData(vtkInformation* req, vtkInformationVector** inInfo, vtkInformationVector* outInfo)
    override;

  int UseScalerForZ;
  int SubSampleStepSize;
};

#endif // smtk_vtk_DEMToMesh_h
