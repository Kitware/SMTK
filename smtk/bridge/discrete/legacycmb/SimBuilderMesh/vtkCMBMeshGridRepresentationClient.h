//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBMeshGridRepresentationClient
// .SECTION Description
// Operator that reads in the simulation mesh on the server and creates a
// vtkCMBMeshGridRepresentation to be used by the model.


#ifndef __vtkCMBMeshGridRepresentationClient_h
#define __vtkCMBMeshGridRepresentationClient_h

#include "vtkObject.h"
#include "cmbSystemConfig.h"


class vtkDiscreteModel;
class vtkSMProxy;

class VTK_EXPORT vtkCMBMeshGridRepresentationClient : public vtkObject
{
public:
  static vtkCMBMeshGridRepresentationClient * New();
  vtkTypeMacro(vtkCMBMeshGridRepresentationClient,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set if the mesh should be used as the simultation input mesh.
  vtkGetMacro(MeshIsAnalysisGrid, int);
  vtkSetMacro(MeshIsAnalysisGrid, int);

  // Description:
  // Set the file name to save the mesh out too.
  // Note: Setting the file name will cause the mesh to be written out
  vtkGetStringMacro(GridFileName);
  vtkSetStringMacro(GridFileName);

  // Description:
  // Source proxy for mesh representation. Default is null, and the mesh representation
  // will be built later in vtkCMBMeshGridRepresentionServer. However, if this is set,
  // it will be passed to vtkCMBMeshGridRepresentionServer as its mesh representation.
  vtkGetObjectMacro(MeshRepresentationSource, vtkSMProxy);
  void SetMeshRepresentationSource(vtkSMProxy* );

  // Description:
  // Reads in the file on the server. Returns true if the operation was successful.
  bool Operate(vtkDiscreteModel *model, vtkSMProxy* serverMeshProxy);

protected:
  vtkCMBMeshGridRepresentationClient();
  virtual ~vtkCMBMeshGridRepresentationClient();

private:
  vtkCMBMeshGridRepresentationClient(const vtkCMBMeshGridRepresentationClient&);  // Not implemented.
  void operator=(const vtkCMBMeshGridRepresentationClient&);  // Not implemented.

    // Description:
  // Flag to indicate that after saving the mesh to file we also want to set this mesh
  // as the grid info representation
  int MeshIsAnalysisGrid;

  char* GridFileName;
  vtkSMProxy* MeshRepresentationSource;

};

#endif
