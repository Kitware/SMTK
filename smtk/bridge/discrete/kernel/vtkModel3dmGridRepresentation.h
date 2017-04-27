//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModel3dmGridRepresentation - Discrete Model representation of an analysis grid from a 3dm grid.
// .SECTION Description
// A class used to provide all of the information that a discrete Model needs
// to keep track of mapping grid objects from the geometry grid to the
// analysis grid.  The source of this information is a 3dm volumetric grid.
// This class assumes that the analysis grid and the
// model grid share the same boundary grid points and a direct
// correspondence between boundary cells and analysis cell boundaries.
// It also assumes that the boundary mesh is either lines, tris, or quads.
// Currently it does not handle floating edges.

#ifndef __smtkdiscrete_vtkModel3dmGridRepresentation_h
#define __smtkdiscrete_vtkModel3dmGridRepresentation_h

#include "smtk/bridge/discrete/kernel/vtkSMTKDiscreteModelModule.h" // For export macro
#include "vtkModelGridRepresentation.h"

class vtkIdTypeArray;
class vtkCharArray;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModel3dmGridRepresentation : public vtkModelGridRepresentation
{
public:
  static vtkModel3dmGridRepresentation* New();
  vtkTypeMacro(vtkModel3dmGridRepresentation, vtkModelGridRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // See vtkModelGridRepresentation.
  virtual bool GetBCSNodalAnalysisGridPointIds(
    vtkDiscreteModel* model, vtkIdType bcsGroupId, int bcGroupType, vtkIdList* pointIds);

  // Description:
  // See vtkModelGridRepresentation.
  virtual bool GetFloatingEdgeAnalysisGridPointIds(
    vtkDiscreteModel* model, vtkIdType modelEdgeId, vtkIdList* pointIds);

  // Description:
  // See vtkModelGridRepresentation.
  virtual bool GetModelEdgeAnalysisPoints(
    vtkDiscreteModel* model, vtkIdType edgeId, vtkIdTypeArray* edgePoints);

  // Description:
  // See vtkModelGridRepresentation.
  virtual bool GetBoundaryGroupAnalysisFacets(
    vtkDiscreteModel* model, vtkIdType boundaryGroupId, vtkIdList* cellIds, vtkIdList* cellSides);

  // Description:
  // Do some type of validation of the mapping information in model.
  // So far we can't guarantee that this works.
  virtual bool IsModelConsistent(vtkDiscreteModel* model);

  // Description:
  // Initialize the information from a given 3dm file. Returns true for success.
  bool Initialize(const char* fileName, vtkDiscreteModel* model,
    vtkIdTypeArray* modelPointToAnalysisPoint, vtkIdTypeArray* modelCellToAnalysisCells,
    vtkCharArray* modelCellToAnalysisCellSides);

  // Description:
  // Set GridFileName to NULL and clear the analysis grid info.
  void Reset();

protected:
  vtkModel3dmGridRepresentation();
  virtual ~vtkModel3dmGridRepresentation();

  friend class vtkCMBModelWriterV2;
  friend class vtkCMBModelWriterV5;
  vtkGetMacro(ModelPointToAnalysisPoint, vtkIdTypeArray*);
  vtkGetMacro(ModelCellToAnalysisCells, vtkIdTypeArray*);
  vtkGetMacro(ModelCellToAnalysisCellSides, vtkCharArray*);

private:
  vtkModel3dmGridRepresentation(const vtkModel3dmGridRepresentation&); // Not implemented.
  void operator=(const vtkModel3dmGridRepresentation&);                // Not implemented.

  vtkIdTypeArray* ModelPointToAnalysisPoint;
  vtkIdTypeArray* ModelCellToAnalysisCells;
  vtkCharArray* ModelCellToAnalysisCellSides;
};
#endif
