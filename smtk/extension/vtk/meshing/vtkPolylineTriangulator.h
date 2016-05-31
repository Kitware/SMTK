//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_vtk_vtkPolylineTriangulator_h
#define __smtk_vtk_vtkPolylineTriangulator_h

#include "smtk/extension/vtk/meshing/Exports.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class vtkCMBMeshServerLauncher;

/// Turn polylines describing facet boundaries into triangulated facets.
class VTKSMTKMESHINGEXT_EXPORT vtkPolylineTriangulator : public vtkPolyDataAlgorithm
{
public:
  static vtkPolylineTriangulator* New();
  vtkTypeMacro(vtkPolylineTriangulator,vtkPolyDataAlgorithm);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The name of a cell array defined on the input polylines
  // describing which facet a polyline belongs to.
  //
  // If NULL (the default), then each polyline defines a separate facet.
  // If an empty string, then all polylines form a single facet.
  // If a valid cell-data array name, then each facet may be composed of
  // multiple polylines as specified by this array.
  //
  // The latter options allow for facets with holes to be defined.
  vtkGetStringMacro(ModelFaceArrayName);
  vtkSetStringMacro(ModelFaceArrayName);

  // Description:
  // Set/get the Remus mesh server launcher instance used to triangulate
  // model faces. If NULL when RequestData() is invoked, one will be
  // created, set, and reused on successive calls to RequestData().
  // However, if you are instantiating multiple vtkPolylineTriangulator
  // instances, performance will improve if they share a launcher.
  virtual void SetLauncher(vtkCMBMeshServerLauncher* launcher);
  vtkGetObjectMacro(Launcher,vtkCMBMeshServerLauncher);

protected:
  vtkPolylineTriangulator();
  virtual ~vtkPolylineTriangulator();

  virtual int FillInputPortInformation(
    int port, vtkInformation* info);

  virtual int RequestData(
    vtkInformation* req,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);

  char* ModelFaceArrayName;
  vtkCMBMeshServerLauncher* Launcher;

private:
  vtkPolylineTriangulator(const vtkPolylineTriangulator&); // Not implemented.
  void operator = (const vtkPolylineTriangulator&); // Not implemented.
};

#endif // __vtkPolylineTriangulator_h
