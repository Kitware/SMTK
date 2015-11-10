//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtkdiscrete_vtkRegionsToLoops_h
#define __smtkdiscrete_vtkRegionsToLoops_h

#include "smtk/bridge/discrete/extension/meshing/vtkSMTKDiscreteMeshingExtModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

namespace smtk {
  namespace bridge {
    namespace discrete {


// Limitations:
// This does not handle non-manifold loops where a vertex has more than 2 line
// segments attached.
class VTKSMTKDISCRETEMESHINGEXT_EXPORT vtkRegionsToLoops : public vtkPolyDataAlgorithm
{
public:
  static vtkRegionsToLoops* New();
  vtkTypeMacro(vtkRegionsToLoops,vtkPolyDataAlgorithm);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkRegionsToLoops();
  virtual ~vtkRegionsToLoops();

  virtual int FillInputPortInformation(
    int port, vtkInformation* info);

  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector** inputInfo,
    vtkInformationVector* outputInfo);

private:
  vtkRegionsToLoops(const vtkRegionsToLoops&); // Not implemented.
  void operator = (const vtkRegionsToLoops&); // Not implemented.
};
    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif // __vtkRegionsToLoops_h
