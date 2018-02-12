//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBMeshGridRepresentationOperation
// .SECTION Description
// Operation that reads in the simulation mesh on the server and creates a
// vtkCMBMeshGridRepresentation to be used by the model.

#ifndef __vtkCMBMeshGridRepresentationOperation_h
#define __vtkCMBMeshGridRepresentationOperation_h

#include "cmbSystemConfig.h"
#include "vtkObject.h"
#include "vtkWeakPointer.h"

class vtkCMBMeshWrapper;
class vtkAlgorithm;

class VTK_EXPORT vtkCMBMeshGridRepresentationOperation : public vtkObject
{
public:
  static vtkCMBMeshGridRepresentationOperation* New();
  vtkTypeMacro(vtkCMBMeshGridRepresentationOperation, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Sets OperateSucceeded.
  void Operate(vtkCMBMeshWrapper* meshWrapper);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

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
  void SetMeshRepresentationInput(vtkAlgorithm* meshSource);

protected:
  vtkCMBMeshGridRepresentationOperation();
  virtual ~vtkCMBMeshGridRepresentationOperation();

private:
  vtkCMBMeshGridRepresentationOperation(
    const vtkCMBMeshGridRepresentationOperation&);              // Not implemented.
  void operator=(const vtkCMBMeshGridRepresentationOperation&); // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;

  // Description:
  // Flag to indicate that after saving the mesh to file we also want to set this mesh
  // as the grid info representation
  int MeshIsAnalysisGrid;

  char* GridFileName;
  vtkWeakPointer<vtkAlgorithm> MeshRepresentationSource;
};

#endif
