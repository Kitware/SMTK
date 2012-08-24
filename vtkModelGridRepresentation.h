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
// .NAME vtkModelGridRepresentation - Abstract class for Discrete model representation of an analysis grid.
// .SECTION Description
// An abstract class used to provide all of the information that a discrete model needs
// to keep track of mapping grid objects from the geometry grid to the
// analysis grid.

#ifndef __vtkModelGridRepresentation_h
#define __vtkModelGridRepresentation_h

#include "vtkDiscreteModelModule.h" // For export macro
#include <vtkObject.h>

class vtkDiscreteModel;
class vtkIdList;
class vtkIdTypeArray;

class VTKDISCRETEMODEL_EXPORT vtkModelGridRepresentation : public vtkObject
{
public:
  vtkTypeMacro(vtkModelGridRepresentation,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetStringMacro(GridFileName);
  vtkSetStringMacro(GridFileName);

  // Description:
  // Get the point ids with respect to the analysis grid of the given nodal group.
  // Does checking to make sure that the information is valid and returns true if successful.
  virtual bool GetBCSNodalAnalysisGridPointIds(vtkDiscreteModel* model, vtkIdType bcsGroupId,
    int bcGroupType, vtkIdList* pointIds) = 0;

  // Description:
  // Get the point ids with respect to the analysis grid of the given floating edge.
  // Does checking to make sure that the information is valid and returns true if successful.
  virtual bool GetFloatingEdgeAnalysisGridPointIds(vtkDiscreteModel* model, vtkIdType modelEdgeId,
                                                   vtkIdList* pointIds) = 0;

  // Description:
  // Get the point ids with respect to the analysis grid of all of the points
  // classified on the model edge with id edgeId.
  virtual bool GetModelEdgeAnalysisPoints(vtkDiscreteModel* model, vtkIdType edgeId,
                                          vtkIdTypeArray* edgePoints) = 0;

  // Description:
  // Get the model cell info with respect to the analysis grid of the given boundary group.
  // It only returns one of the cells which are adjacent to the model cell.  Does checking
  // to make sure that the information is valid and returns true if successful.
  // cellIds are in fortran style ordering and cell sides are between 1 and 4 for tets.
  // This is meant for 3D models.
  virtual bool GetBoundaryGroupAnalysisFacets(vtkDiscreteModel* model, vtkIdType boundaryGroupId,
                                              vtkIdList* cellIds, vtkIdList* cellSides) = 0;

   // Description:
  // Do some type of validation of the mapping information in model.
  // So far we can't guarantee that this works.
  virtual bool IsModelConsistent(vtkDiscreteModel* model) = 0;

  // Description:
  // Reset the object back to an unloaded state.
  virtual void Reset();

  // Description:
  // Check whether the current ModelInfoFileName is the same as the given filename
  virtual bool IsSameModelInfoFile(const char* filename);

protected:
  vtkModelGridRepresentation();
  virtual ~vtkModelGridRepresentation();

  vtkGetStringMacro(ModelInfoFileName);
  vtkSetStringMacro(ModelInfoFileName);

private:
  vtkModelGridRepresentation(const vtkModelGridRepresentation&);  // Not implemented.
  void operator=(const vtkModelGridRepresentation&);  // Not implemented.

  // Description:
  // The name of the analysis grid file.
  char* GridFileName;

  // Description:
  // The name of the file containing the cmb model info, which is relating the mesh
  // back to the model from where the mesh is generated.
  char* ModelInfoFileName;
};
#endif

