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
// .NAME vtkModel3dm2DGridRepresentation - Discrete Model representation of an analysis grid from a 2D 3dm grid.
// .SECTION Description
// A class used to provide all of the information that a CMBModel needs
// to keep track of mapping grid objects from the geometry grid to the
// analysis grid.  The source of this information is a 3dm surface/2D grid.
// This class assumes that the analysis grid and the model grid share the
// same boundary grid points.  We also assume that the indexing of the
// master polydata is the same as the 3dm grid it was created from.
// Currently it does not handle floating edges.

#ifndef __smtkcmb_vtkModel3dm2DGridRepresentation_h
#define __smtkcmb_vtkModel3dm2DGridRepresentation_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelGridRepresentation.h"


class vtkIdTypeArray;
class vtkCharArray;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModel3dm2DGridRepresentation : public vtkModelGridRepresentation
{
public:
  static vtkModel3dm2DGridRepresentation* New();
  vtkTypeMacro(vtkModel3dm2DGridRepresentation,vtkModelGridRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // See vtkModelGridRepresentation.
  virtual bool GetBCSNodalAnalysisGridPointIds(vtkDiscreteModel* model,
    vtkIdType bcsGroupId, int bcGroupType,  vtkIdList* pointIds);

  // Description:
  // See vtkModelGridRepresentation.
  virtual bool GetFloatingEdgeAnalysisGridPointIds(vtkDiscreteModel* model, vtkIdType modelEdgeId,
                                                   vtkIdList* pointIds);

  // Description:
  // See vtkModelGridRepresentation.
  virtual bool GetModelEdgeAnalysisPoints(vtkDiscreteModel* model, vtkIdType edgeId,
                                          vtkIdTypeArray* edgePoints);

  // Description:
  // See vtkModelGridRepresentation.
  virtual bool GetBoundaryGroupAnalysisFacets(vtkDiscreteModel* model, vtkIdType boundaryGroupId,
                                              vtkIdList* cellIds, vtkIdList* cellSides);

  virtual bool IsModelConsistent(vtkDiscreteModel*)
  {
    return true;
  }

protected:
  vtkModel3dm2DGridRepresentation();
  virtual ~vtkModel3dm2DGridRepresentation();

private:
  vtkModel3dm2DGridRepresentation(const vtkModel3dm2DGridRepresentation&);  // Not implemented.
  void operator=(const vtkModel3dm2DGridRepresentation&);  // Not implemented.
};
#endif

