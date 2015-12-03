//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModel3dm2DGridRepresentation - Discrete Model representation of an analysis grid from a 2D 3dm grid.
// .SECTION Description
// A class used to provide all of the information that a CMBModel needs
// to keep track of mapping grid objects from the geometry grid to the
// analysis grid.  The source of this information is a 3dm surface/2D grid.
// This class assumes that the analysis grid and the model grid share the
// same boundary grid points.  We also assume that the indexing of the
// master polydata is the same as the 3dm grid it was created from.
// Currently it does not handle floating edges.

#ifndef __smtkdiscrete_vtkModel3dm2DGridRepresentation_h
#define __smtkdiscrete_vtkModel3dm2DGridRepresentation_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
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

