//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkRayIntersectionLocator - Use vtkCellTree to find <emph>all</emph> ray intersections.

// .SECTION Description
// Use vtkCellTree's data structure to find all intersections of a ray with a
// polygonal dataset.

// .SECTION Caveats
//

// .SECTION See Also
// vtkLocator vtkCellTreeLocator

#ifndef __smtk_vtk_vtkRayIntersectionLocator_h
#define __smtk_vtk_vtkRayIntersectionLocator_h

#include "smtk/extension/vtk/meshing/vtkSMTKMeshingExtModule.h" // For export macro
#include "vtkCellTreeLocator.h"
#include "vtkVector.h"
#include <vector> // Needed for public interface.

class VTKSMTKMESHINGEXT_EXPORT vtkRayIntersectionLocator : public vtkCellTreeLocator
{
public:
  static vtkRayIntersectionLocator* New();
  vtkTypeMacro(vtkRayIntersectionLocator, vtkCellTreeLocator);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  virtual int AllIntersectionsAlongSegment(const vtkVector3d& p0, const vtkVector3d& p1,
    std::vector<vtkVector3d>& points, std::vector<double>& params,
    std::vector<vtkVector3d>& pcoords, std::vector<vtkIdType>& cellIds, std::vector<int>& subIds);

  // Description:
  // Reimplemented to support bad compilers
  int IntersectWithLine(const double a0[3], const double a1[3], double tol, double& t, double x[3],
    double pcoords[3], int& subId, vtkIdType& cellId, vtkGenericCell* cell) override;

  // Description:
  // Reimplemented to support bad compilers
  int IntersectWithLine(const double p1[3], const double p2[3], double tol, double& t, double x[3],
    double pcoords[3], int& subId) override
  {
    return this->Superclass::IntersectWithLine(p1, p2, tol, t, x, pcoords, subId);
  }

  // Description:
  // Reimplemented to support bad compilers
  int IntersectWithLine(const double p1[3], const double p2[3], double tol, double& t, double x[3],
    double pcoords[3], int& subId, vtkIdType& cellId) override
  {
    return this->Superclass::IntersectWithLine(p1, p2, tol, t, x, pcoords, subId, cellId);
  }

  // Description:
  // Reimplemented to support bad compilers
  int IntersectWithLine(
    const double p1[3], const double p2[3], vtkPoints* points, vtkIdList* cellIds) override
  {
    return this->Superclass::IntersectWithLine(p1, p2, points, cellIds);
  }

  // Description:
  // Reimplemented to support bad compilers
  vtkIdType FindCell(double x[3]) override { return this->Superclass::FindCell(x); }

protected:
  vtkRayIntersectionLocator();
  ~vtkRayIntersectionLocator();

private:
  vtkRayIntersectionLocator(const vtkRayIntersectionLocator&); // Not implemented.
  void operator=(const vtkRayIntersectionLocator&);            // Not implemented.
};

#endif // __vtkRayIntersectionLocator_h
