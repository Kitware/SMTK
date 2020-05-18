//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_vtk_geometry_Geometry_h
#define smtk_vtk_geometry_Geometry_h

#include "smtk/extension/vtk/geometry/vtkSMTKGeometryExtModule.h"

#include "smtk/extension/vtk/geometry/Backend.h"
#include "smtk/geometry/GeometryForBackend.h"

#include "vtkDataObject.h"
#include "vtkSmartPointer.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace geometry
{

/**\brief A base class for geometry providers that will supply VTK data.
  *
  * In addition to the data itself, VTK needs some information about
  * the context of the data:
  * + its dimension (point, curve, surface, volume) so that render order
  *   can prevent as much z-fighting as possible;
  * + the purpose of mapping (does the object's geometry represent a
  *   surface to be drawn, a glyph position/orientation/size, a texture
  *   map, etc.). Users may override the presentation _style_ to suit
  *   their needs, but not the _purpose_ which the data serves.
  * This class defines additional virtual methods that providers
  * must implement so the backend can query for the information above.
  */
class VTKSMTKGEOMETRYEXT_EXPORT Geometry
  : public smtk::geometry::GeometryForBackend<vtkSmartPointer<vtkDataObject>>
{
public:
  using DataType = vtkSmartPointer<vtkDataObject>;
  smtkTypeMacro(smtk::extension::vtk::geometry::Geometry);
  smtkSuperclassMacro(smtk::geometry::GeometryForBackend<DataType>);
  Geometry() = default;
  virtual ~Geometry() = default;

  /// The contextual purpose of geometry supplied by the provider.
  enum Purpose
  {
    Surface, //!< Geometry is a set of cells representing a simulation manifold.
    Glyph,   //!< Geometry is a set of points with scalars representing orientation, scale, etc.
    Label //!< Geometry has a cell-scalar that indicates classification, possibly of multiple objects.
  };

  const Backend& backend() const override { return m_backend; }

  /// The VTK backend requires the parametric dimension of each object's geometry.
  ///
  /// This is used to order rendering so that things of lower dimension are
  /// rendered to the z-buffer before objects of higher dimension (giving them
  /// priority so they are not obscured).
  virtual int dimension(const smtk::resource::PersistentObjectPtr& obj) const = 0;

  /// The VTK backend requires a purpose for each object's geometry.
  virtual Purpose purpose(const smtk::resource::PersistentObjectPtr& obj) const = 0;

  /// A convenience to add a field-data color array to a cache entry (used to set object color).
  static void addColorArray(
    vtkDataObject* data,
    const std::vector<double>& color,
    const std::string& name = "entity color");

protected:
  Backend m_backend;
};

} // namespace geometry
} // namespace vtk
} // namespace extension
} // namespace smtk

#endif // smtk_vtk_geometry_Geometry_h
