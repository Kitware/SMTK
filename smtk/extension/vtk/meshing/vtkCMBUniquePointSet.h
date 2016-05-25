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

#ifndef __smtk_vtk_vtkCMBUniquePointSet_
#define __smtk_vtk_vtkCMBUniquePointSet_

#include "smtk/extension/vtk/meshing/Exports.h" // For export macro

#include <map>
#include <vector>
#include <utility>
#include <cstddef> // for size_t
#include "vtkType.h"
#include "vtkABI.h"

namespace smtk {
  namespace vtk {

class VTKSMTKMESHINGEXT_EXPORT vtkCMBUniquePointSet
  {
  public:
    struct InternalPt
      {
      InternalPt(double _x, double _y):x(_x),y(_y){}
      bool operator<(const InternalPt& r) const
        {
        return this->x != r.x ? (this->x < r.x) : (this->y < r.y);
        }
      double x,y;
      };

    vtkCMBUniquePointSet():numPts(0){}

    vtkIdType numPts;

    vtkIdType addPoint(const double& x, const double& y)
    {
      typedef std::map<InternalPt,vtkIdType>::const_iterator c_it;
      InternalPt pt = InternalPt(x,y);
      c_it foundPt = pt2ptId.find(pt);
      if(foundPt == pt2ptId.end())
        {
        ptId2pt.push_back(pt);
        foundPt = pt2ptId.insert(
                std::pair<InternalPt,vtkIdType>(pt,numPts++)).first;
        }
      return foundPt->second;
    }

    vtkIdType addPoint(const double* p)
      {return this->addPoint(p[0],p[1]);}

    vtkIdType getPointId(const double& x, const double& y) const
    {
      typedef std::map<InternalPt,vtkIdType>::const_iterator c_it;
      c_it foundPtId = pt2ptId.find(InternalPt(x,y));
      return foundPtId == pt2ptId.end() ? -1 : foundPtId->second;
    }

    vtkIdType getPointId(double* p) const
      {return this->getPointId(p[0],p[1]);}

    bool getPoint(const vtkIdType& ptId, double& x, double& y) const
      {
      if(static_cast<size_t>(ptId) >= ptId2pt.size())
        {
        return false;
        }
      x = ptId2pt[ptId].x;
      y = ptId2pt[ptId].y;
      return true;
      }

    bool getPoint(const vtkIdType& ptId, double* pt) const
      {return this->getPoint(ptId,pt[0],pt[1]);}

    int getNumberOfPoints() const {return static_cast<int>(ptId2pt.size());}

  private:
    //BTX
    std::map<InternalPt,vtkIdType> pt2ptId;
    std::vector<InternalPt> ptId2pt;
    //ETX
  };
  } // namespace vtk
} // namespace smtk

#endif
