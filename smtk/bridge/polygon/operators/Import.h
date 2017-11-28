//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_polygon_Import_h
#define __smtk_session_polygon_Import_h

#include "smtk/bridge/polygon/Operator.h"
#include "smtk/bridge/polygon/Resource.h"

class vtkIdTypeArray;
class vtkPolyData;

namespace smtk
{
namespace bridge
{
namespace polygon
{

/**\brief Import a CMB polygon model file.
  *
  * The supported file extensions currently:
  *    "Moab files (*.h5m *.sat *.brep *.stp *.cub *.exo)
  *
  * NOT YET:
  *    "VTK data files (*.vtk *.vtu *.vtp)
  *    "Solids (*.2dm *.3dm *.sol *.stl *.tin *.obj)
  *    "SimBuilder files (*.crf *.sbt *.sbi *.sbs)
  *    "Map files (*.map)
  *    "Poly files (*.poly *.smesh)
  *    "Shape files (*.shp)
  */
class SMTKPOLYGONSESSION_EXPORT Import : public Operator
{
public:
  smtkTypeMacro(Import);
  smtkCreateMacro(Import);
  smtkSharedFromThisMacro(smtk::operation::NewOp);
  smtkSuperclassMacro(Operator);

  bool ableToOperate() override;

protected:
  Import();
  smtk::model::OperatorResult operateInternal() override;
  virtual const char* xmlDescription() const override;
  int taggedPolyData2PolygonModelEntities(smtk::bridge::polygon::Resource::Ptr& resource,
    vtkIdTypeArray* tagInfo, vtkPolyData* mesh, smtk::model::Model& model);
  int basicPolyData2PolygonModelEntities(
    smtk::bridge::polygon::Resource::Ptr& resource, vtkPolyData* mesh, smtk::model::Model& model);
};

} // namespace polygon
} // namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_Import_h
