//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME PythonExportGridInfo3D.h - 3D grid representation for smtk interface
// .SECTION Description
// .SECTION See Also

#ifndef __PythonExportGridInfo3D_h
#define __PythonExportGridInfo3D_h

#include "PythonExportGridInfo.h"

class vtkDiscreteModel;

class PythonExportGridInfo3D : public PythonExportGridInfo
{
  typedef smtk::model::GridInfo::ApiStatus ApiStatus;

public:
  /// Constructor & destructor
  PythonExportGridInfo3D(vtkDiscreteModel* model);
  virtual ~PythonExportGridInfo3D();

  /// Returns analysis grid cells for specified model entity.
  virtual std::vector<int> analysisGridCells(int modelEntityId, ApiStatus& status);

  /// Returns "grid items" for the geometry on the boundary of a model entity.
  virtual std::vector<std::pair<int, int> > boundaryItemsOf(int modelEntityId, ApiStatus& status);

  /// Returns "grid items" for the geometry of a model entity that is on
  //  the boundary of a next-higher-dimension model entity.
  virtual std::vector<std::pair<int, int> > asBoundaryItems(
    int modelEntityId, int boundedModelEntityId, ApiStatus& status);

protected:
  virtual std::string getClassName() const; // for status messages
};

#endif /* __PythonExportGridInfo3D_h */
