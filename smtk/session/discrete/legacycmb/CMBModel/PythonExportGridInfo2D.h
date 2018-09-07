//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME PythonExportGridInfo2D.h - 2D grid representation for smtk interface
// .SECTION Description
// .SECTION See Also

#ifndef __PythonExportGridInfo2D_h
#define __PythonExportGridInfo2D_h

#include "PythonExportGridInfo.h"

class vtkDiscreteModel;

class PythonExportGridInfo2D : public PythonExportGridInfo
{
  typedef smtk::model::GridInfo::ApiStatus ApiStatus;

public:
  /// Constructor & destructor
  PythonExportGridInfo2D(vtkDiscreteModel* model);
  virtual ~PythonExportGridInfo2D();

  /// Returns analysis grid cells for specified model entity.
  virtual std::vector<int> analysisGridCells(int modelEntityId, ApiStatus& status);

  /// Returns "grid items" for the geometry on the boundary of a model entity.
  virtual std::vector<std::pair<int, int> > boundaryItemsOf(int modelEntityId, ApiStatus& status);

  /// Returns "grid items" for the geometry of a model entity that is on
  //  the boundary of a next-higher-dimension model entity.
  virtual std::vector<std::pair<int, int> > asBoundaryItems(
    int modelEntityId, int boundedModelEntityId, ApiStatus& status);

  /// Returns set of grid point id pairs representing the grid edges
  //  for an input bounddary group id.
  //  For interim use exporting 2D grids for AdH output.
  virtual std::vector<std::pair<int, int> > edgeGridItems(int boundaryGroupId, ApiStatus& status);

protected:
  virtual std::string getClassName() const; // for status messages
};

#endif /* __PythonExportGridInfo2D_h */
