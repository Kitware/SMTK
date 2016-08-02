//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_vtk_DEMToMesh_h
#define __smtk_vtk_DEMToMesh_h

#include "smtk/extension/vtk/filter/Exports.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class VTKSMTKFILTEREXT_EXPORT vtkDEMToMesh : public vtkPolyDataAlgorithm
{
public:
  static vtkDEMToMesh* New();
  vtkTypeMacro(vtkDEMToMesh,vtkPolyDataAlgorithm);

  void SetUseScalerForZ(int v);

protected:
  vtkDEMToMesh();
  virtual ~vtkDEMToMesh();

  virtual int FillInputPortInformation(int port, vtkInformation* info);

  virtual int RequestData(vtkInformation* req,
                          vtkInformationVector** inInfo,
                          vtkInformationVector* outInfo);

  int UseScalerForZ;
  int SubSampleStepSize;

private:
  vtkDEMToMesh(const vtkDEMToMesh&); // Not implemented.
  void operator = (const vtkDEMToMesh&); // Not implemented.
};

#endif // __smtk_vtk_DEMToMesh_h
