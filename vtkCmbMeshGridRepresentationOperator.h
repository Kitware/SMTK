/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME vtkCmbMeshGridRepresentationOperator
// .SECTION Description
// Operator that reads in the simulation mesh on the server and creates a
// vtkCmbMeshGridRepresentation to be used by the model.

#ifndef __vtkCmbMeshGridRepresentationOperator_h
#define __vtkCmbMeshGridRepresentationOperator_h

#include "vtkObject.h"
#include "vtkWeakPointer.h"

class vtkCmbMeshWrapper;
class vtkAlgorithm;

class VTK_EXPORT vtkCmbMeshGridRepresentationOperator : public vtkObject
{
public:
  static vtkCmbMeshGridRepresentationOperator * New();
  vtkTypeRevisionMacro(vtkCmbMeshGridRepresentationOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Sets OperateSucceeded.
  void Operate(vtkCmbMeshWrapper* meshWrapper);

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
  // will be built later in vtkCmbMeshGridRepresentionServer. However, if this is set,
  // it will be passed to vtkCmbMeshGridRepresentionServer as its mesh representation.
  void SetMeshRepresentationInput(vtkAlgorithm* meshSource);

protected:
  vtkCmbMeshGridRepresentationOperator();
  virtual ~vtkCmbMeshGridRepresentationOperator();

private:

  vtkCmbMeshGridRepresentationOperator(const vtkCmbMeshGridRepresentationOperator&);  // Not implemented.
  void operator=(const vtkCmbMeshGridRepresentationOperator&);  // Not implemented.

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
