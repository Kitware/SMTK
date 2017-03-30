//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBUniquePointSet - Unique point storage class
// .SECTION Description
// This class is for quickly organizing a bunch of points such that there
// are no duplicates

#ifndef __smtk_vtk_cmbUniquePointSet_
#define __smtk_vtk_cmbUniquePointSet_

#include "smtk/common/CompilerInformation.h"    //needed for SMTK_MSVC flag
#include "smtk/extension/vtk/meshing/Exports.h" // For export macro

#include "vtkABI.h"
#include "vtkType.h"

struct cmbUniquePointSet_InternalPt
{
  cmbUniquePointSet_InternalPt()
    : x(0.)
    , y(0.)
  {
  }
  cmbUniquePointSet_InternalPt(double _x, double _y)
    : x(_x)
    , y(_y)
  {
  }
  bool operator<(const cmbUniquePointSet_InternalPt& r) const
  {
    return this->x != r.x ? (this->x < r.x) : (this->y < r.y);
  }
  double x, y;
};

class VTKSMTKMESHINGEXT_EXPORT cmbUniquePointSet
{
public:
  typedef cmbUniquePointSet_InternalPt InternalPt;

  cmbUniquePointSet();
  ~cmbUniquePointSet();

  vtkIdType addPoint(const double& x, const double& y);
  vtkIdType addPoint(const double* p);
  vtkIdType getPointId(const double& x, const double& y) const;
  vtkIdType getPointId(double* p) const;
  bool getPoint(const vtkIdType& ptId, double& x, double& y) const;
  bool getPoint(const vtkIdType& ptId, double* pt) const;
  int getNumberOfPoints() const;

private:
  struct Internals;
  Internals* Internal;
};

#endif
