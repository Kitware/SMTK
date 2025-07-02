//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_polygon_Import_h
#define smtk_session_polygon_Import_h

#include "smtk/session/polygon/Operation.h"
#include "smtk/session/polygon/Resource.h"

class vtkIdTypeArray;
class vtkPolyData;

namespace smtk
{
namespace session
{
namespace polygon
{

/**\brief Import a CMB polygon model file.
  *
  * NOT YET:
  *    "VTK data files (*.vtk *.vtu *.vtp)
  *    "Solids (*.2dm *.3dm *.sol *.stl *.tin *.obj)
  *    "SimBuilder files (*.crf *.sbt *.sbi *.sbs)
  *    "Map files (*.map)
  *    "Poly files (*.poly *.smesh)
  *    "Shape files (*.shp)
  */
class SMTKPOLYGONSESSION_EXPORT Import : public Operation
{
public:
  smtkTypeMacro(smtk::session::polygon::Import);
  smtkCreateMacro(Import);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

  bool ableToOperate() override;

protected:
  Import();
  Result operateInternal() override;
  const char* xmlDescription() const override;
  int taggedPolyData2PolygonModelEntities(
    smtk::session::polygon::Resource::Ptr& resource,
    vtkIdTypeArray* tagInfo,
    vtkPolyData* mesh,
    smtk::model::Model& model);
  int basicPolyData2PolygonModelEntities(
    smtk::session::polygon::Resource::Ptr& resource,
    vtkPolyData* mesh,
    smtk::model::Model& model);
};

} // namespace polygon
} // namespace session
} // namespace smtk

#endif // smtk_session_polygon_Import_h
