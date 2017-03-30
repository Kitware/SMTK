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

#include "smtk/extension/vtk/meshing/cmbUniquePointSet.h"

#include <cstddef> // for size_t
#include <map>
#include <utility>
#include <vector>

struct cmbUniquePointSet::Internals
{
  vtkIdType numPts;
  std::map<InternalPt, vtkIdType> pt2ptId;
  std::vector<InternalPt> ptId2pt;
};

cmbUniquePointSet::cmbUniquePointSet()
{
  this->Internal = new Internals();
  this->Internal->numPts = 0;
}

cmbUniquePointSet::~cmbUniquePointSet()
{
  delete this->Internal;
}

vtkIdType cmbUniquePointSet::addPoint(const double& x, const double& y)
{
  typedef std::map<InternalPt, vtkIdType>::const_iterator c_it;
  InternalPt pt = InternalPt(x, y);
  c_it foundPt = this->Internal->pt2ptId.find(pt);
  if (foundPt == this->Internal->pt2ptId.end())
  {
    this->Internal->ptId2pt.push_back(pt);
    foundPt =
      this->Internal->pt2ptId.insert(std::pair<InternalPt, vtkIdType>(pt, this->Internal->numPts++))
        .first;
  }
  return foundPt->second;
}

vtkIdType cmbUniquePointSet::addPoint(const double* p)
{
  return this->addPoint(p[0], p[1]);
}

vtkIdType cmbUniquePointSet::getPointId(const double& x, const double& y) const
{
  typedef std::map<InternalPt, vtkIdType>::const_iterator c_it;
  c_it foundPtId = this->Internal->pt2ptId.find(InternalPt(x, y));
  return foundPtId == this->Internal->pt2ptId.end() ? -1 : foundPtId->second;
}

vtkIdType cmbUniquePointSet::getPointId(double* p) const
{
  return this->getPointId(p[0], p[1]);
}

bool cmbUniquePointSet::getPoint(const vtkIdType& ptId, double& x, double& y) const
{
  if (static_cast<size_t>(ptId) >= this->Internal->ptId2pt.size())
  {
    return false;
  }
  x = this->Internal->ptId2pt[ptId].x;
  y = this->Internal->ptId2pt[ptId].y;
  return true;
}

bool cmbUniquePointSet::getPoint(const vtkIdType& ptId, double* pt) const
{
  return this->getPoint(ptId, pt[0], pt[1]);
}

int cmbUniquePointSet::getNumberOfPoints() const
{
  return static_cast<int>(this->Internal->ptId2pt.size());
}
