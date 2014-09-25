/*=========================================================================

Copyright (c) 2013 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME PythonExportGridInfo.h - Abstract grid representation for smtk interface
// .SECTION Description
// Abstract base class for PythonGridInfo2D and PythonGridInfo3D classes
// .SECTION See Also

#ifndef __PythonExportGridInfo_h
#define __PythonExportGridInfo_h

#include "smtk/model/GridInfo.h"

class vtkDiscreteModel;
class vtkModelGeneratedGridRepresentation;

class PythonExportGridInfo : public smtk::model::GridInfo
{
  typedef smtk::model::GridInfo::ApiStatus ApiStatus;

 public:
  /// Constructor & destructor
  PythonExportGridInfo(vtkDiscreteModel *model);
  virtual ~PythonExportGridInfo();

  // Returns highest dimension elements/cells in the grid
  virtual int dimension(ApiStatus& status) const;

  /// Returns the type of cell for the specified analysis grid cell id.
  virtual int cellType(int gridCellId, ApiStatus& status);

  /// Returns the grid point ids for a specified model entity id.
  virtual std::vector<int>
  pointIds(int modelEntityId, PointClosure closure, ApiStatus& status);

  /// Returns the grid point ids for a specified grid cell id.
  //  The points are ordered using the VTK convention.
  virtual std::vector<int>
  cellPointIds(int gridCellId, ApiStatus& status);

  /// Returns the dimensional coordinates of a specified grid point id.
  virtual std::vector<double>
  pointLocation(int gridPointId, ApiStatus& status);

  /// Returns the classification id for the node or element set for a
  //  specified model entity. The function applies to MOAB-style grids.
  virtual std::string
  nodeElemSetClassification(int modelEntityId, ApiStatus& status);

  /// Returns the classification id for the mesh side set for a specified
  //  model entity. The function applies to MOAB-style grids.
  virtual std::string
  sideSetClassification(int modelEntityId, ApiStatus& status);

  /// Returns set of grid point id pairs representing the grid edges
  //  for an input bounddary group id.
  //  For interim use exporting 2D grids for AdH output.
  virtual std::vector<std::pair<int, int> >
  edgeGridItems(int boundaryGroupId, ApiStatus& status);

protected:
  vtkModelGeneratedGridRepresentation *getGeneratedGridRep(ApiStatus& status);
  virtual std::string getClassName() const = 0;  // for status messages
  vtkDiscreteModel *m_model;
};

#endif  /* __PythonExportGridInfo_h */
