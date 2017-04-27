//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_vtk_vtkRegionsToLoops_h
#define __smtk_vtk_vtkRegionsToLoops_h

#include "smtk/extension/vtk/meshing/Exports.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

// Limitations:
// This does not handle non-manifold loops where a vertex has more than 2 line
// segments attached.
class VTKSMTKMESHINGEXT_EXPORT vtkRegionsToLoops : public vtkPolyDataAlgorithm
{
public:
  static vtkRegionsToLoops* New();
  vtkTypeMacro(vtkRegionsToLoops, vtkPolyDataAlgorithm);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkRegionsToLoops();
  virtual ~vtkRegionsToLoops();

  virtual int FillInputPortInformation(int port, vtkInformation* info);

  virtual int RequestData(
    vtkInformation* request, vtkInformationVector** inputInfo, vtkInformationVector* outputInfo);

private:
  vtkRegionsToLoops(const vtkRegionsToLoops&); // Not implemented.
  void operator=(const vtkRegionsToLoops&);    // Not implemented.
};

#endif // __vtkRegionsToLoops_h
